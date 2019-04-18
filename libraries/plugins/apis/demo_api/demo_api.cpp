#include <steem/plugins/demo_api/demo_api.hpp>
#include <steem/plugins/demo_api/demo_api_plugin.hpp>

#include <steem/utilities/git_revision.hpp>

#include <steem/chain/util/reward.hpp>
#include <steem/chain/util/uint256.hpp>

#include <fc/git_revision.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/thread/future.hpp>
#include <boost/thread/lock_guard.hpp>


namespace steem{
namespace plugins{
namespace demo{

namespace detail{
    class demo_api_impl {
        public:
        demo_api_impl() {}
        ~demo_api_impl() {}

        get_reviewer_return get_reviewer(const get_reviewer_args &args) const {
            get_reviewer_return final;
            if (args.arg != 0) {
                final.reviewer1 = "reviewera";
                final.reviewer2 = "reviewerb";
                final.reviewer3 = "reviewerc";
                final.reviewer4 = "reviewerd";
                final.reviewer5 = "reviewere";
                final.reviewer6 = "reviewerf";
            }
            
            return final;
        }

    };

}

   demo_api::demo_api() : my(new detail::demo_api_impl()) {
       JSON_RPC_REGISTER_API(STEEM_DEMO_API_PLUGIN_NAME);
   }

   demo_api::~demo_api() {}

   void demo_api_plugin::plugin_initialize(const appbase::variables_map &options) {
       api = std::make_shared<demo_api>();
   }

   DEFINE_LOCKLESS_APIS(demo_api,(get_reviewer))
}
}
}