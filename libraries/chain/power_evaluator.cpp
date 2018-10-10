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

#include <graphene/chain/power_evaluator.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/khc/util.hpp>

namespace graphene { namespace chain {

void_result power_convert_evaluator::do_evaluate( const power_convert_operation& o )
{ try {
   const database& _db = db();
   uint32_t skip = _db.get_node_properties().skip_flags;
   share_type account_amount = _db.get_balance( o.account, o.amount.asset_id ).amount;
   bool insufficient_balance =  account_amount >= o.amount.amount;
   const asset_object& khd_asset_obj = o.refer_amount.asset_id(_db);
   KHC_WASSERT( insufficient_balance,"Account(${account}) Insufficient Balance(${balance}) To Conver (${amount}) Power",
                ("account",o.account)("balance",khc::khc_amount_to_string(account_amount,khd_asset_obj.precision))
                ("amount",khc::khc_amount_to_string(o.amount.amount,khd_asset_obj.precision)) );

   KHC_EASSERT(khd_asset_obj.symbol == KHD_ASSET_SYMBOL,"Account(${account}) Convert Power Refer Asset (${symbol}) is not (${ref_asset})",
               ("account",o.account)("symbol",khd_asset_obj.symbol)("ref_asset",KHD_ASSET_SYMBOL));

   if( skip & database::skip_assert_evaluation )
      return void_result();

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result power_convert_evaluator::do_apply( const power_convert_operation& o )
{ try {
   database& d = db();

   const asset_object& ref_asset = o.refer_amount.asset_id(d);
   const asset_bitasset_data_object& current_bitasset_data = ref_asset.bitasset_data(d);

   auto khd_price = current_bitasset_data.current_feed.settlement_price;
   KHC_WASSERT( !khd_price.is_null(),"khd price is null,need to publish khd feed price first!" );

   share_type power_amount = (fc::uint128_t(o.amount.amount.value) / CONVERT_POWER_REFER_ASSET_RATE
                              / khd_price.base.amount.value * khd_price.quote.amount.value).to_uint64();

   const account_power_index& balance_index = d.get_index_type<account_power_index>();
   auto range = balance_index.indices().get<by_account_power_from>().equal_range(boost::make_tuple(o.account));

   bool found = false;
   for (const account_power_object& power_object : boost::make_iterator_range(range.first, range.second))
   {
       //all power
       if(power_object.power_from!=khc::power_from_melt){
           continue;
       }
       found = true;
       d.modify(power_object, [&](account_power_object& a) {
          a.power_value += power_amount;
       });
       break;
   }

   if(!found)
   {
       d.create<account_power_object>([&](account_power_object& s)
       {
           s.owner = o.account;
           s.power_from = graphene::khc::power_from_melt;
           s.power_value = power_amount;
       });
   }


   current_bitasset_data.current_feed.validate();

   //adjust balance
   d.adjust_balance( o.account, -o.amount );

   d.adjust_balance( GRAPHENE_NULL_ACCOUNT, o.amount );

   khc_dlog("account(${account}) convert ${khc_amount} KHC to ${power_amount} POWER.",
            ("account",o.account)("khc_amount",khc::khc_amount_to_string(o.amount.amount,GRAPHENE_BLOCKCHAIN_PRECISION_DIGITS))
            ("power_amount",khc::khc_amount_to_string(power_amount,GRAPHENE_BLOCKCHAIN_PRECISION_DIGITS)));

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

} } // graphene::chain
