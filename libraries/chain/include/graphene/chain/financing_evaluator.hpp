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
#pragma once

#include <graphene/chain/evaluator.hpp>

namespace graphene { namespace chain {

class asset_investment_evaluator;
class issue_asset_to_investors_evaluator;
struct asset_investment_operation;
struct issue_asset_to_investors_operation;;
class refund_investment_evaluator;
struct refund_investment_operation;
class claim_asset_investment_evaluator;
struct claim_asset_investment_operation;

class asset_investment_evaluator : public evaluator<asset_investment_evaluator>
{
    public:
        typedef asset_investment_operation operation_type;

        void_result do_evaluate( const asset_investment_operation& op );
        void_result do_apply( const asset_investment_operation& op );
};

class issue_asset_to_investors_evaluator : public evaluator<issue_asset_to_investors_evaluator>
{
    public:
        typedef issue_asset_to_investors_operation operation_type;

        share_type total_issue;
        const asset_dynamic_data_object* asset_dyn_data = nullptr;
        std::vector<const asset_investment_object*>           investment_objects;
        std::vector<share_type>           issue_amounts;

        void_result do_evaluate( const issue_asset_to_investors_operation& op );
        void_result do_apply( const issue_asset_to_investors_operation& op );
};

class refund_investment_evaluator : public evaluator<refund_investment_evaluator>
{
    public:
        typedef refund_investment_operation operation_type;

        void_result do_evaluate( const refund_investment_operation& op );
        void_result do_apply( const refund_investment_operation& op );
};

class claim_asset_investment_evaluator : public evaluator<claim_asset_investment_evaluator>
{
    public:
        typedef claim_asset_investment_operation operation_type;

        void_result do_evaluate( const claim_asset_investment_operation& op);
        void_result do_apply( const claim_asset_investment_operation& op);
};

class investor_claims_token_evaluator : public evaluator<investor_claims_token_evaluator>
{
    public:
        typedef investor_claims_token_operation operation_type;

        share_type tokens;
        const asset_dynamic_data_object *asset_dyn_data = nullptr;
        std::vector<const asset_investment_object *> investment_objects;
        void_result do_evaluate( const investor_claims_token_operation& op);
        void_result do_apply( const investor_claims_token_operation& op);
};

} } // graphene::chain
