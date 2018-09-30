#pragma once

#include <stdint.h>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/object.hpp>
#include <utility>

namespace graphene { namespace khc {
using namespace graphene::chain;

#define CONVERT_POWER_REFER_ASSET "KHD"
#define CONVERT_POWER_REFER_ASSET_RATE 10 //means 10 KHD = 1 Power

enum power_from_source
{
    power_from_all = 0,//all common power minus locked
    power_from_locked = 1,//locked power
    power_from_register = 2,//register account
    power_from_pay = 3,//pay by dapp XJTODO
    power_from_laud = 4,//laud by other members
    power_from_melt = 5, //use khc melt
    power_from_max = 6 //just for judge
};

std::string khc_amount_to_string(share_type amount,uint8_t precision);
share_type khc_amount_from_string(std::string amount_string,uint8_t precision);

std::pair<share_type, share_type> power_required_for_finacing(share_type minimum_financing_amount);


}
}


FC_REFLECT_ENUM( graphene::khc::power_from_source,
                (power_from_all)
                (power_from_locked)
                (power_from_register)
                (power_from_pay)
                (power_from_laud)
                (power_from_melt)
                (power_from_max)
              )