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

    int run(int argc, char **argv)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        STR_t indiURL = this.indiURI();
        STR_t mqttURL = this.mqttURI();
        STR_t redisURL = this.redisURI();

        STR_t mqttUsername = this.mqttUsername();
        STR_t mqttPassword = this.mqttPassword();
        STR_t redisUsername = this.redisUsername();
        STR_t redisPassword = this.redisPassword();

        int nodeTimeoutMS = this.nodeTimeoutMS();

        /*------------------------------------------------------------------------------------------------------------*/

        int opt;

        while((opt = getopt(argc, argv, "i:m:r:u:p:U:P:t:h")) != -1)
        {
            switch (opt)
            {
            case 'i':
                indi_uri = optarg;
                break;
            case 'm':
                mqtt_uri = optarg;
                break;
            case 'r':
                redis_uri = optarg;
                break;
            case 'u':
                mqtt_username = optarg;
                break;
            case 'p':
                mqtt_password = optarg;
                break;
            case 'U':
                redis_username = optarg;
                break;
            case 'P':
                redis_password = optarg;
                break;
            case 't':
                node_timeout = atoi(optarg);
                break;
            case 'h':
            default:
                this.print_usage(
                    argv[0],
                    tcp_uri,
                    mqtt_uri,
                    redis_uri,
                    mqtt_username,
                    mqtt_password,
                    redis_username,
                    redis_password,
                    node_timeout
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
