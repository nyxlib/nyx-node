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
        STR_t indiURL,
        STR_t mqttURL,
        STR_t nssURL,
        STR_t mqttUsername,
        STR_t mqttPassword,
        int node_timeout
    ) {
        std::fprintf(
            stderr,
            "Usage: %s [options]\n"
            "\n"
            "Options:\n"
            "  -i <url>       INDI server URL (default: %s)\n"
            "  -m <url>       MQTT broker URL (default: %s)\n"
            "  -s <url>       Stream server URL (default: %s)\n"
            "  -u <username>  MQTT username (default: %s)\n"
            "  -p <password>  MQTT password (default: %s)\n"
            "  -t <ms>        Node poll timeout (default: %d)\n"
            "  -h             Show this help and exit\n",
            prog,
            indiURL != NULL && indiURL[0] != '\0' ? indiURL : "none",
            mqttURL != NULL && mqttURL[0] != '\0' ? mqttURL : "none",
            nssURL != NULL && nssURL[0] != '\0' ? nssURL : "none",
            mqttUsername != NULL && mqttUsername[0] != '\0' ? mqttUsername : "none",
            mqttPassword != NULL && mqttPassword[0] != '\0' ? mqttPassword : "none",
            node_timeout
        );
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    int run(int argc, char **argv)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        STR_t indiURL = this->indiURL();
        STR_t mqttURL = this->mqttURL();
        STR_t nssURL = this->nssURL();

        STR_t mqttUsername = this->mqttUsername();
        STR_t mqttPassword = this->mqttPassword();

        int nodeTimeoutMS = this->nodeTimeoutMS();

        /*------------------------------------------------------------------------------------------------------------*/

        int opt;

        while((opt = getopt(argc, argv, "i:m:s:u:p:t:h")) != -1)
        {
            switch (opt)
            {
            case 'i':
                indiURL = optarg;
                break;
            case 'm':
                mqttURL = optarg;
                break;
            case 's':
                nssURL = optarg;
                break;
            case 'u':
                mqttUsername = optarg;
                break;
            case 'p':
                mqttPassword = optarg;
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
                    nssURL,
                    mqttUsername,
                    mqttPassword,
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
            nssURL,
            mqttUsername,
            mqttPassword,
            nullptr,
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

    virtual STR_t indiURL() const = 0;
    virtual STR_t mqttURL() const = 0;
    virtual STR_t nssURL() const = 0;

    virtual STR_t mqttUsername() const = 0;
    virtual STR_t mqttPassword() const = 0;

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
