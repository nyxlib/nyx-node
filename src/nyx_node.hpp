/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_NODE_HPP
#define NYX_NODE_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include <vector>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <csignal>

#include "nyx_node.h"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace Nyx {

/*--------------------------------------------------------------------------------------------------------------------*/
/* BaseDevice                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

class BaseDevice
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    virtual ~BaseDevice() = default;

    /*----------------------------------------------------------------------------------------------------------------*/

    virtual void glueInitialize() = 0;

    virtual void initialize(nyx_node_t *node) = 0;

    virtual void finalize(nyx_node_t *node) = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    virtual STR_t name() const = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    inline const std::vector<nyx_dict_t *> &vectors() const
    {
        return m_vectors;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

protected:
    /*----------------------------------------------------------------------------------------------------------------*/

    inline void registerVector(nyx_dict_t *vector)
    {
        m_vectors.push_back(vector);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    std::vector<nyx_dict_t *> m_vectors;

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/
/* BaseDriver                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

class BaseDriver
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    virtual ~BaseDriver() = default;

    /*----------------------------------------------------------------------------------------------------------------*/

    static void print_usage(
        STR_t prog,
        STR_t indi_uri,
        STR_t mqtt_uri,
        STR_t redis_uri,
        STR_t mqtt_username,
        STR_t mqtt_password,
        STR_t redis_username,
        STR_t redis_password,
        int node_timeout
    ) {
        std::fprintf(
            stderr,
            "Usage: %s [options]\n"
            "\n"
            "Options:\n"
            "  -i URI   INDI server URI (default: %s)\n"
            "  -m URI   MQTT broker URI (default: %s)\n"
            "  -r URI   Redis server URI (default: %s)\n"
            "  -u USER  MQTT username (default: %s)\n"
            "  -p PASS  MQTT password (default: %s)\n"
            "  -U USER  Redis username (default: %s)\n"
            "  -P PASS  Redis password (default: %s)\n"
            "  -t MS    Node poll timeout (default: %d)\n"
            "  -h       Show this help and exit\n",
            prog,
            indi_uri != NULL && indi_uri[0] != '\0' ? indi_uri : "none",
            mqtt_uri != NULL && mqtt_uri[0] != '\0' ? mqtt_uri : "none",
            redis_uri != NULL && redis_uri[0] != '\0' ? redis_uri : "none",
            mqtt_username != NULL && mqtt_username[0] != '\0' ? mqtt_username : "none",
            mqtt_password != NULL && mqtt_password[0] != '\0' ? mqtt_password : "none",
            redis_username != NULL && redis_username[0] != '\0' ? redis_username : "none",
            redis_password != NULL && redis_password[0] != '\0' ? redis_password : "none",
            node_timeout
        );
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    int run(int argc, char **argv)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        STR_t indiURL = this->indiURI();
        STR_t mqttURL = this->mqttURI();
        STR_t redisURL = this->redisURI();

        STR_t mqttUsername = this->mqttUsername();
        STR_t mqttPassword = this->mqttPassword();
        STR_t redisUsername = this->redisUsername();
        STR_t redisPassword = this->redisPassword();

        int nodeTimeoutMS = this->nodeTimeoutMS();

        /*------------------------------------------------------------------------------------------------------------*/

        int opt;

        while((opt = getopt(argc, argv, "i:m:r:u:p:U:P:t:h")) != -1)
        {
            switch (opt)
            {
            case 'i':
                indiURL = optarg;
                break;
            case 'm':
                mqttURL = optarg;
                break;
            case 'r':
                redisURL = optarg;
                break;
            case 'u':
                mqttUsername = optarg;
                break;
            case 'p':
                mqttPassword = optarg;
                break;
            case 'U':
                redisUsername = optarg;
                break;
            case 'P':
                redisPassword = optarg;
                break;
            case 't':
                nodeTimeoutMS = std::atoi(optarg);
                break;
            case 'h':
            default:
                BaseDriver::print_usage(
                    argv[0],
                    indiURL,
                    mqttURL,
                    redisURL,
                    mqttUsername,
                    mqttPassword,
                    redisUsername,
                    redisPassword,
                    nodeTimeoutMS
                );

                return (opt == 'h') ? 0 : 1;
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_memory_initialize();

        /*------------------------------------------------------------------------------------------------------------*/

        this->glueInitialize();

        /*------------------------------------------------------------------------------------------------------------*/

        std::vector<nyx_dict_t *> vectors;

        for(const auto &uptr: this->m_devices)
        {
            const auto &subVectors = uptr->vectors();

            vectors.insert(vectors.end(), subVectors.begin(), subVectors.end());
        }

        /*------------------------------------------------------------------------------------------------------------*/

        vectors.push_back(nullptr);

        /*------------------------------------------------------------------------------------------------------------*/

        static volatile sig_atomic_t s_signo = 0;

        auto handler = +[](int signo) {

            s_signo = signo;
        };

        std::signal(SIGINT, handler);
        std::signal(SIGTERM, handler);

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_node_t *node = nyx_node_initialize(
            this->name(),
            vectors.data(),
            indiURL,
            mqttURL,
            mqttUsername,
            mqttPassword,
            nullptr,
            redisURL,
            redisUsername,
            redisPassword,
            3000,
            true
        );

        for(const auto &uptr: this->m_devices) uptr->initialize(node);
        while(s_signo == 0) nyx_node_poll(node, nodeTimeoutMS);
        for(const auto &uptr: this->m_devices) uptr->finalize(node);

        nyx_node_finalize(node, true);

        /*------------------------------------------------------------------------------------------------------------*/

        this->m_devices.clear();

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_memory_finalize();

        /*------------------------------------------------------------------------------------------------------------*/

        return 0;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

protected:
    /*----------------------------------------------------------------------------------------------------------------*/

    virtual void glueInitialize() = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    virtual STR_t name() const = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    virtual STR_t indiURI() const = 0;

    virtual STR_t mqttURI() const = 0;
    virtual STR_t mqttUsername() const = 0;
    virtual STR_t mqttPassword() const = 0;

    virtual STR_t redisURI() const = 0;
    virtual STR_t redisUsername() const = 0;
    virtual STR_t redisPassword() const = 0;

    virtual int nodeTimeoutMS() const = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    inline void registerDevice(std::unique_ptr<BaseDevice> device)
    {
        this->m_devices.emplace_back(std::move(device));
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    std::vector<std::unique_ptr<BaseDevice>> m_devices;

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace Nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_NODE_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/
