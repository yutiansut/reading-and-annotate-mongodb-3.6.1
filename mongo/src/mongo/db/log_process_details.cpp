// @file log_process_details.cpp

/**
*    Copyright (C) 2013 MongoDB, Inc.
*
*    This program is free software: you can redistribute it and/or  modify
*    it under the terms of the GNU Affero General Public License, version 3,
*    as published by the Free Software Foundation.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*    As a special exception, the copyright holders give permission to link the
*    code of portions of this program with the OpenSSL library under certain
*    conditions as described in each individual source file and distribute
*    linked combinations including the program with the OpenSSL library. You
*    must comply with the GNU Affero General Public License in all respects
*    for all of the code used other than as permitted herein. If you modify
*    file(s) with this exception, you may extend this exception to your
*    version of the file(s), but you are not obligated to do so. If you do not
*    wish to do so, delete this exception statement from your version. If you
*    delete this exception statement from all source files in the program,
*    then also delete it in the license file.
*/

#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kControl

#include "mongo/platform/basic.h"

#include "mongo/db/log_process_details.h"

#include "mongo/db/repl/repl_set_config.h"
#include "mongo/db/repl/replication_coordinator.h"
#include "mongo/db/repl/replication_coordinator_global.h"
#include "mongo/db/server_options.h"
#include "mongo/db/server_options_helpers.h"
#include "mongo/util/log.h"
#include "mongo/util/net/sock.h"
#include "mongo/util/net/ssl_manager.h"
#include "mongo/util/processinfo.h"
#include "mongo/util/version.h"

namespace mongo {

bool is32bit() {
    return (sizeof(int*) == 4);
}

/*
2018-03-26T15:07:15.988+0800 I CONTROL  [initandlisten] db version v3.6.1
2018-03-26T15:07:15.988+0800 I CONTROL  [initandlisten] git version: nogitversion
2018-03-26T15:07:15.988+0800 I CONTROL  [initandlisten] allocator: tcmalloc
2018-03-26T15:07:15.988+0800 I CONTROL  [initandlisten] modules: none
2018-03-26T15:07:15.988+0800 I CONTROL  [initandlisten] build environment:
2018-03-26T15:07:15.988+0800 I CONTROL  [initandlisten]     distarch: x86_64
2018-03-26T15:07:15.988+0800 I CONTROL  [initandlisten]     target_arch: x86_64
2018-03-26T15:07:15.988+0800 I CONTROL  [initandlisten] options: { config: "./mongo.conf", net: { bindIpAll: true, maxIncomingConnections: 20000, port: 8002 }, operationProfiling: { mode: "slowOp", slowOpThresholdMs: 100 }, processManagement: { fork: true }, replication: { oplogSizeMB: 20480, replSetName: "363" }, sharding: { archiveMovedChunks: true, clusterRole: "shardsvr" }, storage: { dbPath: "/data1/mongodb/363/data/", directoryPerDB: true, engine: "wiredTiger", journal: { enabled: true }, wiredTiger: { collectionConfig: { blockCompressor: "none" }, engineConfig: { cacheSizeGB: 40.0, directoryForIndexes: true }, indexConfig: { prefixCompression: false } } }, systemLog: { destination: "file", logAppend: true, path: "/data1/mongodb/363/logs/mongod.log", verbosity: 6 } }
*/
void logProcessDetails() {
    auto&& vii = VersionInfoInterface::instance();
    log() << mongodVersion(vii);
    vii.logBuildInfo();

    printCommandLineOpts();
}

void logProcessDetailsForLogRotate() {
    log() << "pid=" << ProcessId::getCurrent() << " port=" << serverGlobalParams.port
          << (is32bit() ? " 32" : " 64") << "-bit "
          << "host=" << getHostNameCached();

    auto replCoord = repl::getGlobalReplicationCoordinator();
    if (replCoord != nullptr &&
        replCoord->getReplicationMode() == repl::ReplicationCoordinator::modeReplSet) {
        auto rsConfig = replCoord->getConfig();

        if (rsConfig.isInitialized()) {
            log() << "Replica Set Config: " << rsConfig.toBSON();
            log() << "Replica Set Member State: " << (replCoord->getMemberState()).toString();
        } else {
            log() << "Node currently has no Replica Set Config.";
        }
    }

    logProcessDetails();
}

}  // mongo