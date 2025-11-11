#pragma once
#include <chrono>
#include <thread>
#include <iostream>
#include <cstdint>

/**
 * @class FPS
 * @brief 指定したFPSレートで処理を制御するためのユーティリティクラス
 *
 * 指定されたFPSに基づいてフレームごとの待機処理を行うことで、
 * 描画などの処理を安定したフレームレートで実行できるようにします。
 */


//Cludeバージョン
//class FPS {
//public:
//    FPS() = delete;
//
//    explicit FPS(uint64_t fps)
//        : m_MicrosecondsPerFrame(1000000 / fps),
//        m_last_time(std::chrono::high_resolution_clock::now())
//    {
//    }
//
//    /**
//     * @brief フレーム開始時に呼び出し、前フレームからの経過時間を取得
//     * @return 経過時間（マイクロ秒）
//     */
//    uint64_t BeginFrame()
//    {
//        auto now = std::chrono::high_resolution_clock::now();
//        auto delta_us = std::chrono::duration_cast<std::chrono::microseconds>(now - m_last_time).count();
//
//        // 異常に小さいまたは大きいdelta timeを制限
//        if (delta_us < 1000) {  // 1ms未満
//            delta_us = 16667;   // 60FPS相当に修正
//        }
//        else if (delta_us > 50000) {  // 50ms超過
//            delta_us = 16667;   // 60FPS相当に修正
//        }
//
//        m_delta_time = delta_us;
//        return m_delta_time;
//    }
//
//    /**
//     * @brief フレーム終了時に呼び出し、必要に応じて待機してFPSを維持
//     */
//    void EndFrame() {
//        auto now = std::chrono::high_resolution_clock::now();
//        auto actual_frame_time = std::chrono::duration_cast<std::chrono::microseconds>(now - m_last_time).count();
//
//        int64_t sleep_us = static_cast<int64_t>(m_MicrosecondsPerFrame) - static_cast<int64_t>(actual_frame_time);
//
//        if (sleep_us > 0) {
//            std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
//        }
//
//        // フレーム完了後にm_last_timeを更新
//        m_last_time = std::chrono::high_resolution_clock::now();
//    }
//
//private:
//    uint64_t m_MicrosecondsPerFrame = 0;
//    uint64_t m_delta_time = 0;
//    std::chrono::high_resolution_clock::time_point m_last_time;
//};
    class FPS {
    public:
        /**
         * @brief デフォルトコンストラクタは禁止
         */
        FPS() = delete;

        /**
         * @brief FPSを指定して初期化
         * @param fps 目標とするフレーム毎秒数
         */
        explicit FPS(uint64_t fps)
            : m_MicrosecondsPerFrame(1000000 / fps),
            m_last_time(std::chrono::high_resolution_clock::now())
        {
        }

        /**
         * @brief 前回のTick()からの経過時間をマイクロ秒単位で取得（m_last_timeは更新しない）
         * @return 経過時間（マイクロ秒）
         */
        uint64_t CalcDelta() {
		    auto now = std::chrono::high_resolution_clock::now();
            auto delta_us = std::chrono::duration_cast<std::chrono::microseconds>(now - m_last_time).count();
            m_delta_time = delta_us; // 更新はしないが、m_delta_timeを更新
		    return m_delta_time;
        }

        /**
         * @brief 残り時間がある場合のみスリープしてFPSを維持
         *
         * @note m_delta_time は Tick() で更新されていることが前提。
         * フレーム処理時間が指定FPSよりも長い場合（sleep_us <= 0）はスリープしません。
         */
        void Wait() const {
            int64_t sleep_us = static_cast<int64_t>(m_MicrosecondsPerFrame) - static_cast<int64_t>(m_delta_time);
            if (sleep_us > 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
            }
            // sleep_us <= 0 ならフレームが遅れているため、スリープは行わない
        }

        /**
         * @brief Tickを呼び出すことで、次のフレームまでの待機とdelta_timeの更新を行う
         *
         * この関数により、前フレームからの経過時間を計測し、目標FPSより短い場合には待機します。
         * 再計測後に m_last_time を更新します。
         */
        void Tick() {
            auto now = std::chrono::high_resolution_clock::now();
            auto delta_us = std::chrono::duration_cast<std::chrono::microseconds>(now - m_last_time).count();
            int64_t sleep_us = static_cast<int64_t>(m_MicrosecondsPerFrame) - static_cast<int64_t>(delta_us);
            if (sleep_us > 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
                now = std::chrono::high_resolution_clock::now(); // スリープ後に再取得
                delta_us = std::chrono::duration_cast<std::chrono::microseconds>(now - m_last_time).count();
            }
            m_last_time = now;
            m_delta_time = delta_us;
        }

        /*
            /// @brief 【旧実装】CalcDelta: 呼び出し時にm_last_timeを更新してdelta_timeを返す
            /// @return 経過時間（マイクロ秒）
            ///
            /// @bug この実装では Wait() 呼び出し前に m_last_time を更新してしまうため、
            /// 実際の待機時間が短く計算され、指定FPS以上に速くなるバグがありました。
            /// Tick() のように、スリープ後に正確な時刻を取得して更新する必要があります。

            uint64_t CalcDelta() {
                auto now = std::chrono::steady_clock::now();
                auto delta_us = std::chrono::duration_cast<std::chrono::microseconds>(now - m_last_time).count();
                m_last_time = now;
                m_delta_time = delta_us;
                return m_delta_time;
            }
        */

        /*
            /// @brief 【旧実装】Wait: 前回のCalcDeltaで得たm_delta_timeに基づいて待機
            ///
            /// @bug CalcDelta() が m_last_time を更新してしまうため、
            /// Wait() では古い情報を使ってしまい正確な待機時間が得られませんでした。
            /// また、Tick() が導入されたことで処理の整合性が保たれるように改善されました。

            void Wait() const {
                int64_t sleep_us = static_cast<int64_t>(m_MicrosecondsPerFrame) - static_cast<int64_t>(m_delta_time);
                if (sleep_us > 0) {
        #if defined(DEBUG) || defined(_DEBUG)
                    std::cout << "Sleep time: " << sleep_us / 1000.0f << " ms" << std::endl;
        #endif
                }
                std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
            }
        */

    private:
        uint64_t m_MicrosecondsPerFrame = 0;  ///< 1フレームあたりの目標時間（マイクロ秒）
        uint64_t m_delta_time = 0;            ///< 前フレームからの経過時間（マイクロ秒）
        std::chrono::high_resolution_clock::time_point m_last_time;
    };
