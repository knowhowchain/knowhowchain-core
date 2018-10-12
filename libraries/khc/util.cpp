#include <graphene/khc/util.hpp>
#include <graphene/khc/config.hpp>
#include <graphene/chain/protocol/asset.hpp>

uint64_t g_khc_project_asset_financing_cycle_unit = KHC_PROJECT_ASSET_FINANCING_CYCLE_UNIT;
uint64_t g_khc_project_asset_project_cycle_unit = KHC_PROJECT_ASSET_PROJECT_CYCLE_UNIT;

namespace graphene { namespace khc {

std::string khc_amount_to_string(share_type amount,uint8_t precision)
{
   share_type scaled_precision =  graphene::chain::asset::scaled_precision(precision);

   std::string result = fc::to_string(amount.value / scaled_precision.value);
   auto decimals = amount.value % scaled_precision.value;
   if( decimals )
      result += "." + fc::to_string(scaled_precision.value + decimals).erase(0,1);
   return result;
}

share_type khc_amount_from_string(string amount_string,uint8_t precision)
{ try {
   bool negative_found = false;
   bool decimal_found = false;
   for( const char c : amount_string )
   {
      if( isdigit( c ) )
         continue;

      if( c == '-' && !negative_found )
      {
         negative_found = true;
         continue;
      }

      if( c == '.' && !decimal_found )
      {
         decimal_found = true;
         continue;
      }

      FC_THROW( (amount_string) );
   }

   share_type satoshis = 0;

   FC_ASSERT( precision < 19 );
   share_type scaled_precision = scaled_precision_lut[ precision ];;

   const auto decimal_pos = amount_string.find( '.' );
   const string lhs = amount_string.substr( negative_found, decimal_pos );
   if( !lhs.empty() )
      satoshis += fc::safe<int64_t>(std::stoll(lhs)) *= scaled_precision;

   if( decimal_found )
   {
      const size_t max_rhs_size = std::to_string( scaled_precision.value ).substr( 1 ).size();

      string rhs = amount_string.substr( decimal_pos + 1 );
      FC_ASSERT( rhs.size() <= max_rhs_size );

      while( rhs.size() < max_rhs_size )
         rhs += '0';

      if( !rhs.empty() )
         satoshis += std::stoll( rhs );
   }

   FC_ASSERT( satoshis <= GRAPHENE_MAX_SHARE_SUPPLY );

   if( negative_found )
      satoshis *= -1;

   return satoshis;
   } FC_CAPTURE_AND_RETHROW( (amount_string) )
}

share_type power_required_for_finacing(share_type minimum_financing_amount)
{
    share_type convert_ratio = KHC_POWER_CONVERT_KHD_RATIO * graphene::chain::asset::scaled_precision(KHD_PRECISION_DIGITS);
    share_type required_power = minimum_financing_amount / convert_ratio;
    if (minimum_financing_amount % convert_ratio != 0)
        ++required_power;
    required_power *= graphene::chain::asset::scaled_precision(KHC_POWER_PRECISION_DIGITS);
    return required_power;
    /*
    if (minimum_financing_amount <= KHC_POWER_GRADE_0_MAX_FUNDRAISING_QUOTA)
        return std::make_pair(KHC_POWER_GRADE_0_MIN_POWER, KHC_POWER_GRADE_0_MAX_POWER);
    if (minimum_financing_amount <= KHC_POWER_GRADE_1_MAX_FUNDRAISING_QUOTA)
        return std::make_pair(KHC_POWER_GRADE_1_MIN_POWER, KHC_POWER_GRADE_1_MAX_POWER);
    if (minimum_financing_amount <= KHC_POWER_GRADE_2_MAX_FUNDRAISING_QUOTA)
        return std::make_pair(KHC_POWER_GRADE_2_MIN_POWER, KHC_POWER_GRADE_2_MAX_POWER);
    if (minimum_financing_amount <= KHC_POWER_GRADE_3_MAX_FUNDRAISING_QUOTA)
        return std::make_pair(KHC_POWER_GRADE_3_MIN_POWER, KHC_POWER_GRADE_3_MAX_POWER);
    if (minimum_financing_amount <= KHC_POWER_GRADE_4_MAX_FUNDRAISING_QUOTA)
        return std::make_pair(KHC_POWER_GRADE_4_MIN_POWER, KHC_POWER_GRADE_4_MAX_POWER);
    if (minimum_financing_amount <= KHC_POWER_GRADE_5_MAX_FUNDRAISING_QUOTA)
        return std::make_pair(KHC_POWER_GRADE_5_MIN_POWER, KHC_POWER_GRADE_5_MAX_POWER);
    return std::make_pair(KHC_POWER_GRADE_6_MIN_POWER, KHC_POWER_GRADE_6_MAX_POWER);
    */
}

}}
