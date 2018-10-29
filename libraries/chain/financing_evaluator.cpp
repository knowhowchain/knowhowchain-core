/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <graphene/chain/financing_evaluator.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/khc/util.hpp>
#include <graphene/khc/config.hpp>
#include <graphene/chain/is_authorized_asset.hpp>

namespace graphene { namespace chain {

void_result asset_investment_evaluator::do_evaluate( const asset_investment_operation& o )
{ try {
   database& d = db();
   const asset_object& khd_asset_object = d.get(o.amount.asset_id);
   KHC_EASSERT(khd_asset_object.symbol == KHD_ASSET_SYMBOL,"investment must use ${asset}",("asset",KHD_ASSET_SYMBOL));
   KHC_WASSERT(o.fee.amount >= 0,"invalid asset investment fee amount");
   KHC_WASSERT(o.amount.amount > 0,"invalid investment amount:${investment_amount}",
               ("investment_amount",khc::khc_amount_to_string(o.amount.amount.value,khd_asset_object.precision)));

   KHC_WASSERT(is_authorized_asset(d, o.account_id(d), o.investment_asset_id(d)));

   asset account_khd_asset = d.get_balance(d.get(o.account_id), khd_asset_object);
   KHC_WASSERT( account_khd_asset >= o.amount,"account amount(${account_amount}) less than investment amount(${investment_amount})",
                ("account_amount",khc::khc_amount_to_string(account_khd_asset.amount.value,khd_asset_object.precision))
                ("investment_amount",khc::khc_amount_to_string(o.amount.amount.value,khd_asset_object.precision)));

   const asset_object& asset_obj = d.get(o.investment_asset_id);
   KHC_WASSERT(!asset_obj.is_market_issued());
   KHC_WASSERT(o.investment_asset_id != asset_id_type(0),"can't investment ${asset}",("asset",GRAPHENE_SYMBOL));
   KHC_WASSERT(asset_obj.is_public_offering(),"asset must be a public offering type.");

   const asset_dynamic_data_object* asset_dyn_data = &asset_obj.dynamic_asset_data_id(d);
   //KHC_WASSERT(asset_dyn_data->state == asset_dynamic_data_object::project_state::financing); XJTODO status need wait maintain

   //amount
   KHC_WASSERT(asset_dyn_data->financing_current_supply < asset_obj.proj_options.max_financing_amount,
               "financing end,current_financing_amount:${current_financing_amount},financing_amount:${max_financing_amount}",
               ("current_financing_amount",asset_dyn_data->financing_current_supply)
               ("max_financing_amount",asset_obj.proj_options.max_financing_amount));

   const auto& dpo = d.get_dynamic_global_properties();
   const auto& po = d.get_global_properties();
   uint32_t start_block_num = asset_obj.proj_options.start_financing_block_num;
   uint32_t end_block_num = asset_obj.proj_options.start_financing_block_num + (asset_obj.proj_options.financing_cycle/po.parameters.block_interval);
   uint32_t curr_block_num = dpo.head_block_number;
   //time
   KHC_WASSERT(curr_block_num>=asset_obj.proj_options.start_financing_block_num&&curr_block_num<=end_block_num,
               "current block num(${curr_block_num}) need betweent [${start_block_num},${end_block_num}]",
               ("curr_block_num",curr_block_num)("start_block_num",start_block_num)("end_block_num",end_block_num));

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result asset_investment_evaluator::do_apply( const asset_investment_operation& o )
{ try {
   database& d = db();
   const auto& dpo = d.get_dynamic_global_properties();
   uint32_t curr_block_num = dpo.head_block_number;
   const asset_object& asset_obj = d.get(o.investment_asset_id);
   const asset_dynamic_data_object* asset_dyn_data = &asset_obj.dynamic_asset_data_id(d);
   asset actual_investment_amount = o.amount;

   //advance end finance
   bool advance_end = asset_dyn_data->financing_current_supply+o.amount.amount>=asset_obj.proj_options.max_financing_amount;
   if(advance_end)
   {
       actual_investment_amount.amount = asset_obj.proj_options.max_financing_amount - asset_dyn_data->financing_current_supply;
       d.modify(asset_obj, [&](asset_object& a) {
          a.proj_options.end_financing_block_num = curr_block_num;
          a.proj_options.end_financing_time = time_point_sec(fc::time_point::now());
       });
   }

   d.create<asset_investment_object>([&](asset_investment_object& s)
   {
       s.investment_account_id = o.account_id;
       s.investment_khd_amount = actual_investment_amount;
       s.investment_asset_id = o.investment_asset_id;
       s.investment_height = curr_block_num;
       s.investment_timestamp = d.head_block_time();
       s.return_financing_flag = false;
   });

   d.modify( *asset_dyn_data, [&]( asset_dynamic_data_object& data )
   {
        data.financing_current_supply += actual_investment_amount.amount;
        data.financing_confidential_supply += actual_investment_amount.amount;
        if(advance_end){
            data.state = asset_dynamic_data_object::project_state::financing_lock;
        }
   });

   d.adjust_balance( o.account_id , -actual_investment_amount );

   const asset_object& khd_asset_object = d.get(o.amount.asset_id);
   uint8_t precision = khd_asset_object.precision;
   khc_dlog("account(${account}) investment asset(${asset}) ${investment_khd_amount} KHD.asset financing min amount:${financing_min_amount},"
            "asset financing_max_amount:${financing_max_amount},current investment:${current_investment},",
            ("account",o.account_id)("asset",o.investment_asset_id)
            ("investment_khd_amount",khc::khc_amount_to_string(actual_investment_amount.amount,precision))
            ("financing_min_amount",khc::khc_amount_to_string(asset_obj.proj_options.min_financing_amount,precision))
            ("financing_max_amount",khc::khc_amount_to_string(asset_obj.proj_options.max_financing_amount,precision))
            ("current_investment",khc::khc_amount_to_string(asset_dyn_data->financing_current_supply,precision)));

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result issue_asset_to_investors_evaluator::do_evaluate( const issue_asset_to_investors_operation& o )
{ try {
   database& d = db();

   KHC_WASSERT(o.fee.amount >= 0, "invalid issue_asset_and_get_financing fee amount.");

   const asset_object& investment_asset_object = o.investment_asset_id(d);
   KHC_WASSERT(investment_asset_object.is_public_offering(),"asset must be a public offering type.");
   KHC_WASSERT( !investment_asset_object.is_market_issued(), "Cannot manually issue a market-issued asset." );
   KHC_WASSERT( investment_asset_object.is_financing_end(d), "Financing is not over." );

   const asset_id_type khd_id = d.get_asset_id(KHD_ASSET_SYMBOL);
   asset_dyn_data = &investment_asset_object.dynamic_asset_data_id(d);
   
   const asset_object &investemnt_asset_object = o.investment_asset_id(d);

   KHC_WASSERT(asset_dyn_data->financing_confidential_supply == asset_dyn_data->financing_current_supply,
               "financing_confidential_supply(${financing_confidential_supply}) != financing_current_supply(${financing_current_supply})",
               ("financing_confidential_supply", asset_dyn_data->financing_confidential_supply)("financing_current_supply", asset_dyn_data->financing_current_supply));
   
   KHC_WASSERT(asset_dyn_data->current_supply == 0, "May have already sent tokens to investors.");

   total_issue = (asset(asset_dyn_data->financing_confidential_supply, khd_id) * investemnt_asset_object.proj_options.khd_exchange_rate * investemnt_asset_object.options.core_exchange_rate).amount;

   const auto &idx = d.get_index_type<asset_investment_index>().indices().get<by_asset>();
   auto range = idx.equal_range(o.investment_asset_id);
   share_type total_investment = 0;
   share_type total_issue_tmp = 0;
   this->investment_objects.clear();
   std::vector<const asset_investment_object *> empty;
   this->investment_objects.swap(empty);
   this->issue_amounts.clear();
   std::vector<share_type> share_empty;
   this->issue_amounts.swap(share_empty);
   std::for_each(range.first, range.second,
                 [&](const asset_investment_object &obj) {
                     KHC_WASSERT(is_authorized_asset(d, obj.investment_account_id(d), obj.investment_asset_id(d)));
                     total_investment += obj.investment_khd_amount.amount;
                     this->investment_objects.push_back(&obj);
                     share_type issue_amount = (fc::uint128_t(total_issue.value) * obj.investment_khd_amount.amount.value / asset_dyn_data->financing_confidential_supply.value).to_uint64();
                     this->issue_amounts.push_back(issue_amount);
                     total_issue_tmp += issue_amount;
                 });

   KHC_WASSERT(total_issue_tmp <= total_issue, "Financing failed, Calculated value：${total_to_issue}, total_issue: ${total_issue}.",
               ("total_to_issue", total_issue_tmp)("total_issue", total_issue));
   issue_amounts[issue_amounts.size()] += (total_issue - total_issue_tmp);

   KHC_WASSERT(total_investment == asset_dyn_data->financing_confidential_supply, "The total amount of financing is not right.total_investment(${total_investment}) !=financing_confidential_supply(${confidential_supply})",
               ("total_investment", total_investment)("total_investment", asset_dyn_data->financing_confidential_supply));
   KHC_WASSERT((asset_dyn_data->current_supply + total_issue) <= investment_asset_object.options.max_supply,
               "Exceeding the maximum supply current_supply($current_supply{}) + total_issue(${total_issue}) != max_supply(${max_supply})",
               ("current_supply", asset_dyn_data->current_supply)("total_issue", total_issue)("max_supply", investment_asset_object.options.max_supply));

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result issue_asset_to_investors_evaluator::do_apply( const issue_asset_to_investors_operation& o )
{ try {
   database& d = db();


   for (decltype(investment_objects.size()) i = 0; i < investment_objects.size(); ++i) {
       d.modify(*(investment_objects[i]), [&](asset_investment_object &obj) {
           obj.has_receive_token = false;
           obj.investment_tokens = issue_amounts[i];
       });
   }

   d.modify(*asset_dyn_data, [&](asset_dynamic_data_object &data) {
       data.current_supply += total_issue;
       data.investment_confidential_supply = total_issue;
       data.investment_current_supply = total_issue;
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result refund_investment_evaluator::do_evaluate( const refund_investment_operation& o )
{ try {
   database& d = db();

   const asset_object& asset_o = d.get(o.investment_asset_id);
   KHC_WASSERT(asset_o.is_public_offering(),"asset must be a public offering type.");
   asset_dynamic = &(asset_o.dynamic_asset_data_id(d));
   KHC_WASSERT(asset_dynamic->state == asset_dynamic_data_object::project_state::financing_failue,"asset project is not in failed state.");

   share_type total_investment(0);
   bool investment_flag(false);
   this->investment_objects.clear();
   std::vector<const asset_investment_object *> empty;
   this->investment_objects.swap(empty);
   const auto& idx = d.get_index_type<asset_investment_index>().indices().get<by_account>();
   auto range = idx.equal_range(o.account_id);
   std::for_each(range.first,range.second,
                 [&](const asset_investment_object& obj){

       if(obj.investment_asset_id == o.investment_asset_id && !obj.return_financing_flag){
            total_investment += obj.investment_khd_amount.amount;
            this->investment_objects.push_back(&obj);
       }else if(obj.investment_asset_id == o.investment_asset_id && obj.return_financing_flag){
            investment_flag = true;
       }
   });

   KHC_EASSERT(asset_dynamic->financing_current_supply - total_investment >= 0,
               "asset financing_current_supply(${current}) have not enough to refund account(${account}) total_investment(${investment}).",
               ("current",asset_dynamic->financing_current_supply)("account",o.account_id)("investment",total_investment));
   if(investment_flag){
       KHC_WASSERT(this->investment_objects.size() > 0,"account had refund investment in asset(${asset}) already.",("asset",asset_o.symbol));
   }else{
       KHC_WASSERT(this->investment_objects.size() > 0,"account has no investment to refund in asset(${asset})",("asset",asset_o.symbol));
   }


   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result refund_investment_evaluator::do_apply( const refund_investment_operation& o )
{ try {
   database& d = db();

   for( const auto investment_object : investment_objects) {
           d.modify( *asset_dynamic, [&]( asset_dynamic_data_object& data ){
                data.financing_current_supply -= investment_object->investment_khd_amount.amount;
           });

           d.adjust_balance(o.account_id,investment_object->investment_khd_amount);
           d.modify( *investment_object, [&]( asset_investment_object& s ){
                s.return_financing_flag = true;
           });
   }
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result claim_bitasset_investment_evaluator::do_evaluate( const claim_bitasset_investment_operation& o )
{ try {
   database& d = db();
   const auto& dpo = d.get_dynamic_global_properties();
   const auto& po = d.get_global_properties();

   const asset_object& asset_o = d.get(o.asset_id);
   KHC_WASSERT(asset_o.is_public_offering(),"asset must be a public offering type.");
   dynamic_o = &(d.get(asset_o.dynamic_asset_data_id));
   KHC_WASSERT(dynamic_o->claim_times < 3,"issuer has claim all asset already.");
   KHC_EASSERT(dynamic_o->financing_current_supply > 0,"asset(${asset}) has no investment less.",("asset",asset_o.symbol));
   KHC_WASSERT(asset_o.is_issue_to_investors(d),"before claim bitasset investment, projecter need issue asset to investors at first.");
   KHC_WASSERT(asset_o.issuer == o.account_id,"account(${account} is not the issuer of asset(${asset}))",
               ("account",o.account_id)("asset",asset_o.symbol));

   KHC_WASSERT(dpo.head_block_number >= asset_o.proj_options.end_financing_block_num ,"project is not financing end.");
   auto project_diff = asset_o.proj_options.project_cycle / po.parameters.block_interval;

   share_type claim_amount(0);
   if(dynamic_o->claim_times == 0){
       auto first_claim_block = asset_o.proj_options.end_financing_block_num;
       KHC_WASSERT(dpo.head_block_number >= first_claim_block,"now_block(${block}),next claim block(${nblock})",
                   ("block",dpo.head_block_number)("nblock",first_claim_block));
       claim_amount =  (fc::uint128_t(dynamic_o->financing_confidential_supply.value) * KHC_FIRST_CLAIM_INVESTMENT_RATIO / KHC_100_PERCENT).to_uint64();
       claim_times = dynamic_o->claim_times + 1;
   }else if(dynamic_o->claim_times == 1){
       auto second_claim_block = asset_o.proj_options.end_financing_block_num + (project_diff/ 2);
       KHC_WASSERT(dpo.head_block_number >= second_claim_block,"now_block(${block}),next claim block(${nblock})",
                   ("block",dpo.head_block_number)("nblock",second_claim_block));

       claim_amount =  (fc::uint128_t(dynamic_o->financing_confidential_supply.value) * KHC_SECOND_CLAIM_INVESTMENT_TATIO / KHC_100_PERCENT).to_uint64();
       claim_times = dynamic_o->claim_times + 1;
   }else{
       auto end_claim_block = asset_o.proj_options.end_financing_block_num + project_diff;
       KHC_WASSERT(dpo.head_block_number >= end_claim_block,"now_block(${block}),next claim block(${nblock})",
                   ("block",dpo.head_block_number)("nblock",end_claim_block));

       share_type claim_first =  (fc::uint128_t(dynamic_o->financing_confidential_supply.value) * KHC_FIRST_CLAIM_INVESTMENT_RATIO / KHC_100_PERCENT).to_uint64();
       share_type claim_second =  (fc::uint128_t(dynamic_o->financing_confidential_supply.value) * KHC_SECOND_CLAIM_INVESTMENT_TATIO / KHC_100_PERCENT).to_uint64();
       claim_amount = dynamic_o->financing_confidential_supply - claim_first - claim_second;
       claim_times = dynamic_o->claim_times + 1;
   }

   KHC_WASSERT(dynamic_o->financing_current_supply - claim_amount >= 0,"issuer claim amount(${amount}),but asset(${asset}) financing_current_supply(${csupply}) is not enough",
               ("amount",claim_amount)("asset",asset_o.symbol)("csupply",dynamic_o->financing_current_supply));

   asset_id_type khd_id = d.get_asset_id(KHD_ASSET_SYMBOL);
   khd_amount = asset(claim_amount,khd_id);

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result claim_bitasset_investment_evaluator::do_apply( const claim_bitasset_investment_operation& o )
{ try {

   database& d = db();
   d.adjust_balance(o.account_id,khd_amount);
   d.modify( *dynamic_o, [&]( asset_dynamic_data_object& s ){
        s.financing_current_supply -= khd_amount.amount;
        s.claim_times = claim_times;
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result claim_asset_investment_evaluator::do_evaluate( const claim_asset_investment_operation& o )
{ try {
   database& d = db();
   const asset_object& asset_o = d.get(o.asset_id);
   KHC_WASSERT(asset_o.is_public_offering(),"asset must be a public offering type.");
   asset_dyn_data = &(d.get(asset_o.dynamic_asset_data_id));

   tokens = 0;
   const auto &idx = d.get_index_type<asset_investment_index>().indices().get<by_account>();
   auto range = idx.equal_range(o.account_id);
   this->investment_objects.clear();
   std::vector<const asset_investment_object *> empty;
   this->investment_objects.swap(empty);
   std::for_each(range.first, range.second,
                 [&](const asset_investment_object &obj) {
                     KHC_WASSERT(is_authorized_asset(d, obj.investment_account_id(d), obj.investment_asset_id(d)));
                     if(obj.investment_asset_id == o.asset_id)
                     {
                         KHC_WASSERT(obj.has_receive_token == false, "${account} have already claim ${a} assets.", ("account", o.account_id)("a", asset_o.symbol));
                         tokens += obj.investment_tokens;
                         this->investment_objects.push_back(&obj);
                     }
                 });
   KHC_WASSERT(tokens > 0, "${account} is not an investor of ${a} assets.", ("account",o.account_id)("a", asset_o.symbol));
   KHC_EASSERT(tokens <= asset_dyn_data->investment_current_supply, "tokens(${tokens}) exceeds the upper limit investment_current_supply(${s}).",
               ("tokens", tokens)("s", asset_dyn_data->investment_current_supply));

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result claim_asset_investment_evaluator::do_apply( const claim_asset_investment_operation& o )
{ try {
   database& d = db();

   d.adjust_balance( o.account_id, asset(tokens, o.asset_id) );

   d.modify( *asset_dyn_data, [&]( asset_dynamic_data_object& a ){
        a.investment_current_supply -= tokens;
   });

   for( const auto investment_object : investment_objects) {
       d.modify(*investment_object, [&](asset_investment_object &obj) {
           obj.has_receive_token = true;
       });
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }
} } // graphene::chain
