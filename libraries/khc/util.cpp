#include <graphene/khc/util.hpp>
#include <graphene/chain/protocol/asset.hpp>

namespace graphene { namespace khc {

std::string khc_amount_to_string(share_type amount,uint8_t precision)
{
   share_type scaled_precision = 1;
   for( uint8_t i = 0; i < precision; ++i )
      scaled_precision *= 10;
   assert(scaled_precision > 0);

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

std::pair<share_type, share_type> power_required_for_finacing(share_type minimum_financing_amount)
{
    if (minimum_financing_amount <= 1000)
        return std::make_pair(51, 99);
    if (minimum_financing_amount <= 5000)
        return std::make_pair(100, 499);
    if (minimum_financing_amount <= 20000)
        return std::make_pair(500, 1999);
    if (minimum_financing_amount <= 100000)
        return std::make_pair(2000, 4999);
    if (minimum_financing_amount <= 500000)
        return std::make_pair(5000, 9999);
    if (minimum_financing_amount <= 1000000)
        return std::make_pair(10000, 49999);
    return std::make_pair(50000, 0); //0 means No upper limit
}

}}
