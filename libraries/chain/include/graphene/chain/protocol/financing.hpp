#pragma once
#include <graphene/chain/protocol/base.hpp>

namespace graphene { namespace chain {

/**
 * @brief This operation is used to investment a asset
 * @ingroup operations
 *
 * account can investment asset
 */
struct asset_investment_operation : public base_operation
{
   struct fee_parameters_type { share_type fee = 1 * GRAPHENE_BLOCKCHAIN_PRECISION; };

   /// Paid by authorizing_account
   asset fee;

   /// The account to investment
   account_id_type account_id;

   /// The asset to investment
   asset_id_type investment_asset_id;

   /// Amount of KHD to investment
   asset amount;

   account_id_type fee_payer()const { return account_id; }
   void validate()const;
};

/**
 * @brief This operation is used to investment a asset
 * @ingroup operations
 *
 * account can investment asset
 */
struct issue_asset_to_investors_operation : public base_operation
{
   struct fee_parameters_type { share_type fee = 1 * GRAPHENE_BLOCKCHAIN_PRECISION; };

   /// Paid by authorizing_account
   asset fee;

   account_id_type issue;

   asset_id_type investment_asset_id;

   account_id_type fee_payer()const { return issue; }
   void validate()const;
};

/**
* @brief This operation is used to refund investment
* @ingroup operations
*
* account refund investment
*/
struct refund_investment_operation : public base_operation
{
  struct fee_parameters_type { share_type fee = 0 * GRAPHENE_BLOCKCHAIN_PRECISION; };


  /// Paid by authorizing_account
  asset fee;


  /// The account to investment
  account_id_type account_id;

  /// The asset to refund investment
  asset_id_type investment_asset_id;

  account_id_type fee_payer()const { return account_id; }
  void validate()const;
};

}} // graphene::chain

FC_REFLECT( graphene::chain::asset_investment_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::asset_investment_operation,
            (fee)(account_id)(investment_asset_id)(amount) )

FC_REFLECT( graphene::chain::issue_asset_to_investors_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::issue_asset_to_investors_operation,
            (fee)(issue)(investment_asset_id) )

FC_REFLECT( graphene::chain::refund_investment_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::refund_investment_operation,
            (fee)(account_id)(investment_asset_id) )
