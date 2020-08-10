//
// Created by Jason Justman on 2019-09-27.
//

#ifndef LIBATSC3_ATSC3NDKPHYBRIDGE_DEMUXEDPCAPVIRTUALPHY_H
#define LIBATSC3_ATSC3NDKPHYBRIDGE_DEMUXEDPCAPVIRTUALPHY_H

#include "Atsc3LoggingUtils.h"

#include <string.h>
#include <jni.h>
#include <thread>
#include <map>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <list>

#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

using namespace std;

#define DEBUG 1
#include "Atsc3JniEnv.h"
#include "IAtsc3NdkPHYClient.h"

#define MODULE_NAME "intf"

// libatsc3 type imports here
#include <atsc3_utils.h>
#include <atsc3_pcap_type.h>
#include <atsc3_route_package_utils.h>

/*
 * : public libatsc3_Iphy_mockable
 */

class Atsc3NdkPHYBridge_DemuxedPcapVirtualPHY : public IAtsc3NdkPHYClient
{
public:
    Atsc3NdkPHYBridge_DemuxedPcapVirtualPHY(): mbInit(false), mbLoop(false), mbRun(false) {    }

    int Init();
    int Open(int fd, int bus, int addr);
    int Prepare(const char *devinfo, int delim1, int delim2);
    int Tune(int freqKHz, int plpId);
    int TuneMultiplePLP(int freqKhz, vector<int> plpIds);
    int ListenPLP1(int plp1); //by default, we will always listen to PLP0, append additional PLP for listening

    int Stop();
    int Close();
    int Reset();
    int Uninit();

    /* phy callback method(s) */
    int atsc3_rx_callback_f(void*, uint64_t ullUser);

    /*
     * pcap methods
     */

    int atsc3_pcap_replay_open_file(const char *filename);
    int atsc3_pcap_replay_open_file_from_assetManager(const char *filename, AAssetManager *mgr);
    int atsc3_pcap_thread_run();
    int atsc3_pcap_thread_stop();


    void LogMsg(const char *msg);
    void LogMsg(const std::string &msg);
    void LogMsgF(const char *fmt, ...);

    int pinFromRxCaptureThread();
    int pinFromRxProcessingThread();
    int pinFromRxStatusThread();


    int RxThread();
    Atsc3JniEnv* Atsc3_Jni_Capture_Thread_Env = NULL;
    Atsc3JniEnv* Atsc3_Jni_Processing_Thread_Env = NULL;
    Atsc3JniEnv* Atsc3_Jni_Status_Thread_Env = NULL;

private:
    bool mbInit;

    std::thread mhRxThread;

    bool mbLoop, mbRun;

    // statistics
    uint32_t s_ulLastTickPrint;
    uint64_t s_ullTotalBytes = 0;
    uint64_t s_ullTotalPkts;
    unsigned s_uTotalLmts = 0;
    std::map<std::string, unsigned> s_mapIpPort;
    int s_nPrevLmtVer = -1;
    uint32_t s_ulL1SecBase;

    //pcap replay context and locals
    int PcapProducerThreadParserRun();
    int PcapConsumerThreadRun();
    int PcapLocalCleanup();

    AAsset*                         pcap_replay_asset_ref_ptr = NULL;
    char*                           pcap_replay_filename = NULL;
    bool                            pcapThreadShouldRun;

    std::thread                     pcapProducerThreadPtr;
    Atsc3JniEnv*                        atsc3_jni_pcap_producer_thread_env = NULL;
    bool                            pcapProducerShutdown = true;

    std::thread                     pcapConsumerThreadPtr;
    Atsc3JniEnv*                        atsc3_jni_pcap_consumer_thread_env = NULL;
    bool                            pcapConsumerShutdown = true;

    atsc3_pcap_replay_context_t*    atsc3_pcap_replay_context = NULL;
    queue<block_t*>                 pcap_replay_buffer_queue;
    mutex                           pcap_replay_buffer_queue_mutex;
    condition_variable              pcap_replay_condition;

    //alc service monitoring
    vector<int>                     atsc3_slt_alc_additional_services_monitored;


public:
    jobject     global_pcap_asset_manager_ref = NULL;

public:
    void ResetStatstics() {
        s_ulLastTickPrint = 0;
        s_ullTotalBytes = s_ullTotalPkts = 0;
        s_uTotalLmts = 0;
        s_mapIpPort.clear();
        s_nPrevLmtVer = -1;
        s_ulL1SecBase = 0;
    }

public:
    // jni stuff
    JavaVM* mJavaVM = nullptr;    // Java VM
    JNIEnv* mJniEnv = nullptr;    // Jni Environment
    jclass mClsDrvIntf = nullptr; // java At3DrvInterface class

    bool JReady() {
        return mJavaVM && mJniEnv && mClsDrvIntf ? true : false;
    }

    jmethodID mOnLogMsgId = nullptr;  // java class method id

    jmethodID atsc3_rf_phy_status_callback_ID = nullptr;
    jmethodID atsc3_update_rf_bw_stats_ID = nullptr;

    //todo: refactor this out - ala https://gist.github.com/qiao-tw/6e43fb2311ee3c31752e11a4415deeb1

    jclass      jni_java_util_ArrayList = nullptr;
    jmethodID   jni_java_util_ArrayList_cctor = nullptr;
    jmethodID   jni_java_util_ArrayList_add = nullptr;


    std::string get_android_temp_folder();

    //moving to "friend" scope
    void atsc3_update_rf_stats(   int32_t tuner_lock,    //1
                                  int32_t rssi,
                                  uint8_t modcod_valid,
                                  uint8_t plp_fec_type,
                                  uint8_t plp_mod,
                                  uint8_t plp_cod,
                                  int32_t nRfLevel1000,
                                  int32_t nSnr1000,
                                  uint32_t ber_pre_ldpc_e7,
                                  uint32_t ber_pre_bch_e9,
                                  uint32_t fer_post_bch_e6,
                                  uint8_t demod_lock,
                                  uint8_t signal,
                                  uint8_t plp_any,
                                  uint8_t plp_all); //15
    void atsc3_update_rf_bw_stats(uint64_t total_pkts, uint64_t total_bytes, unsigned int total_lmts);

private:
    std::thread atsc3_rxStatusThread;
    void RxStatusThread();
    bool rxStatusThreadShouldRun;
};

#define NDK_PCAP_VIRTUAL_PHY_ERROR(...)   	__LIBATSC3_TIMESTAMP_ERROR(__VA_ARGS__);



#endif //LIBATSC3_ATSC3NDKPHYBRIDGE_DEMUXEDPCAPVIRTUALPHY_H