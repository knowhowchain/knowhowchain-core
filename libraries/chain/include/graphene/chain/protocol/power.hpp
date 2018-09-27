#pragma once
#include <graphene/chain/protocol/base.hpp>

namespace graphene { namespace chain {

/**
 * @brief This operation is used to convert accounts core asset to power
 * @ingroup operations
 *
 * Accounts can convert core asset to power which can use to issue asset
 */
struct power_convert_operation : public base_operation
{
   struct fee_parameters_type { share_type fee = 1 * GRAPHENE_BLOCKCHAIN_PRECISION; };

   /// Paid by authorizing_account
   asset fee;

   /// The account to update
   account_id_type account;

   /// Amount to convert
   asset amount;

   /// Reference asset
   asset refer_amount;


   account_id_type fee_payer()const { return account; }
   void validate()const;
};

}} // graphene::chain

FC_REFLECT( graphene::chain::power_convert_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::power_convert_operation,
            (fee)(account)(amount)(refer_amount) )
