#pragma once
#include <steem/plugins/follow/follow_plugin.hpp>
#include <steem/plugins/json_rpc/json_rpc_plugin.hpp>
#include <appbase/application.hpp>

#define STEEM_GET_API_PLUGIN_NAME "get_api"

namespace steem{
namespace plugins{
namespace get{

   class get_api_plugin : public appbase::plugin<get_api_plugin>
   {

   public:
       get_api_plugin() {}
       virtual ~get_api_plugin() {};

       APPBASE_PLUGIN_REQUIRES(
           (steem::plugins::json_rpc::json_rpc_plugin)
           (steem::plugins::follow::follow_plugin)
           )

       static const std::string &name() {
           static std::string name = STEEM_GET_API_PLUGIN_NAME;
           return name;
       }

    virtual void set_program_options( appbase::options_description &cli, appbase::options_description &cfg ) override {};

    virtual void plugin_initialize( const appbase::variables_map &options ) override;
    virtual void plugin_startup() override {};
    virtual void plugin_shutdown() override {};

    std::shared_ptr< class get_api > api;

    };
   
   

}
}
}