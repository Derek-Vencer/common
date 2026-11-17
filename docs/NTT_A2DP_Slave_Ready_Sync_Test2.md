# v0.9.45_Test2_A2DP_Slave_Ready_Sync — implementation gate

Baseline: e5366ac391f48c4173e9ca07a84b64ccda2ac14b
Scope: A2DP startup only. Preserve Connected Prompt scheduling, mixer, EQ fade and AVRCP volume.

## Verified existing control flow

In services/audio_bt/app_bt_stream.cpp (baseline):
- Near line 4446: a2dp_audio_init(), then app_bt_stream_ibrt_reset_synchronize_status().
- Near line 4466: mobile-link side calls app_tws_ibrt_audio_analysis_start(), app_tws_ibrt_audio_sync_start(), and app_bt_stream_ibrt_audio_master_detect_next_packet_start().
- Near line 4472: IBRT-link side calls app_bt_stream_ibrt_audio_slave_detect_next_packet_start(need_autotrigger).
- Near line 4629: app_bt_stream_trigger_init().
- Near line 4667: af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK).
- Near line 4686: app_bt_stream_trigger_checker_start().

## Safety requirements before enabling the experiment

Do NOT postpone af_stream_start() or block the audio thread: the native BT trigger, decoder, audio DMA, and prompt mixer may depend on this lifecycle.

Identify the native master/slave trigger synchronization completion callback and the slave decoder-buffer-ready condition. The ready signal must be scoped to the active A2DP stream session (generation ID), not merely a stale global bool.

Preferred mechanism: keep stream creation and native TWS trigger unchanged; defer only the first audible A2DP PCM output via an A2DP-only gate (not the prompt mixer), if an isolated PCM path is proven available.

Master gate opens when slave ready arrives OR 500 ms deadline expires. If no IBRT peer is connected, bypass gate. On A2DP stop, role switch, device switch, or new session, invalidate outstanding ready messages and timer. After timeout, do not restart or stop an already running stream; slave rejoins through native sync.

## Validation

1. Dual-ear connected: slave ready within deadline, music audible on both, no underrun.
2. Slave absent: master music starts immediately.
3. Slave late: master starts by 500 ms; slave later rejoins.
4. A2DP stop/restart within wait: no stale ready event.
5. Role switch within wait: no stuck mute.
6. Two phones with connected prompts: both prompts audible on both buds, unchanged timing and mixing.
7. HFP / EQ prompt / OTA and case in/out: no regressions.

Suggested log prefix: [NTT_A2DP_SYNC]; include stream generation, peer role, elapsed ms, ready and timeout cause.

Status: DESIGN ONLY — no firmware synchronization code has been applied or compiled.