###########################################################
# Makefile generated by QMDE for CSRA68100                 
#                                                          
# Project: earbud
# Configuration: debug
# Generated: Mon 15. Jun 19:20:52 2020
#                                                          
# WARNING: Do not edit this file. Any changes will be lost 
#          when the project is rebuilt.                    
#                                                          
###########################################################

BUILDOUTPUT_PATH=depend_debug_qcc512x_qcc302x
BUILD_ID=C:\qtil\ADK_QCC512X_QCC302X_WIN_6.4.2.26\tools/ubuild/maker/buildid/examples/timestamp.py
BUILD_TYPE=DEBUG
CHIP_NAME=QCC3020
CHIP_TYPE=qcc512x_qcc302x
DBG_CORE=app/p1
DEFAULT_LIBS=usb_early_init
DEFS=AV_DEBUG BLUELAB CHIP_LED_BASE_PIO=66 CHIP_NUM_LEDS=6 DEBUG HAVE_1_BUTTON HAVE_2_LEDS HAVE_32BIT_DATA_WIDTH HAVE_THERMISTOR HYDRA HYDRACORE INCLUDE_AV INCLUDE_CHARGER INCLUDE_DFU INCLUDE_GATT INCLUDE_GATT_BATTERY_SERVERx INCLUDE_GATT_GAIA_SERVER INCLUDE_HFP INCLUDE_MICFWD INCLUDE_PROMPTS INCLUDE_SCOFWD INCLUDE_TEMPERATURE INCLUDE_TONES INSTALL_HYDRA_LOG QCC3020_AA_DEV_BRD_R2_AA THERMISTOR_ADC=adcsel_led5 THERMISTOR_DATA_FILE=thermistor_lp4549652p3m.h THERMISTOR_ON=THERMISTOR_PIO_UNUSED USE_BDADDR_FOR_LEFT_RIGHT __KALIMBA__ TRAPSET_CORE=1 TRAPSET_PSU=1 SOUL_BLADE INCLUDE_PTEK
EXTRA_WARNINGS=FALSE
FLASH_CONFIG=..\..\64Mbit_default_flash_config.py
HW_VARIANT=QCC3020-AA_DEV-BRD-R2-AA
IDE_CONFIG=debug
IDE_PROJECT=earbud
INCPATHS=../../../../installed_libs/include/firmware_qcc512x_qcc302x ../../../../installed_libs/include/firmware_qcc512x_qcc302x/app ../../../../installed_libs/include/profiles/default_qcc512x_qcc302x ../../../../installed_libs/include/standard ../../chains C:\qtil\ADK_QCC512X_QCC302X_WIN_6.4.2.26\apps/installed_libs/include/firmware_qcc512x_qcc302x C:\qtil\ADK_QCC512X_QCC302X_WIN_6.4.2.26\apps/installed_libs/include/firmware_qcc512x_qcc302x/app C:\qtil\ADK_QCC512X_QCC302X_WIN_6.4.2.26\apps/installed_libs/include/standard C:\qtil\ADK_QCC512X_QCC302X_WIN_6.4.2.26\audio/kalimba/kymera/common/interface/gen/k32
LIBPATHS=../../../../installed_libs/lib/default_qcc512x_qcc302x/native ../../../../installed_libs/lib/os/qcc512x_qcc302x C:\qtil\ADK_QCC512X_QCC302X_WIN_6.4.2.26\apps/installed_libs/lib/os/qcc512x_qcc302x
LIBS=a2dp anc audio_plugin_common audio_processor avrcp bdaddr byte_utils chain connection cryptovm custom_operator file_list gaia gain_utils gatt gatt_gap_server gatt_heart_rate_server gatt_manager gatt_server hfp input_event_manager logging operators packetiser_helper pio_common region rsa_decrypt rsa_pss_constants rtime rwcp_server sdp_parse service system_clock transport_adaptation transport_manager upgrade usb_device_class vmal
OUTDIR=E:\my_job\Soul\ADK6_4_2_26_example1_QCC3020\apps\applications\earbud\qcc512x_qcc302x\QCC3020-AA_DEV-BRD-R2-AA
OUTPUT=earbud
OUTPUT_TYPE=EXECUTABLE
PRESERVED_LIBS=qcc512x_qcc302x
PRIVATE_DEFS=
PRIVATE_INCPATHS=
STRIP_SYMBOLS=FALSE
SW_VARIANT=


INPUTS=\
    ..\..\1_button.buttonxml \
    ..\..\av_headset.h \
    ..\..\av_headset_a2dp.c \
    ..\..\av_headset_a2dp.h \
    ..\..\av_headset_accelerometer.h \
    ..\..\av_headset_adv_manager.c \
    ..\..\av_headset_adv_manager.h \
    ..\..\av_headset_adv_manager_private.h \
    ..\..\av_headset_anc_tuning.c \
    ..\..\av_headset_anc_tuning.h \
    ..\..\av_headset_at_commands.c \
    ..\..\av_headset_at_commands.h \
    ..\..\av_headset_auth.c \
    ..\..\av_headset_auth.h \
    ..\..\av_headset_av.c \
    ..\..\av_headset_av.h \
    ..\..\av_headset_av_caps.c \
    ..\..\av_headset_av_remote.c \
    ..\..\av_headset_av_volume.c \
    ..\..\av_headset_avrcp.c \
    ..\..\av_headset_avrcp.h \
    ..\..\av_headset_battery.c \
    ..\..\av_headset_battery.h \
    ..\..\av_headset_chain_roles.h \
    ..\..\av_headset_charger.c \
    ..\..\av_headset_charger.h \
    ..\..\av_headset_con_manager.c \
    ..\..\av_headset_con_manager.h \
    ..\..\av_headset_config.c \
    ..\..\av_headset_config.h \
    ..\..\av_headset_conn_rules.c \
    ..\..\av_headset_conn_rules.h \
    ..\..\av_headset_db.db \
    ..\..\av_headset_device.c \
    ..\..\av_headset_device.h \
    ..\..\av_headset_gaia.c \
    ..\..\av_headset_gaia.h \
    ..\..\av_headset_gatt.c \
    ..\..\av_headset_gatt.h \
    ..\..\av_headset_gatt_dis.c \
    ..\..\av_headset_gatt_dis.h \
    ..\..\av_headset_gatt_ptek.c \
    ..\..\av_headset_gatt_ptek.h \
    ..\..\av_headset_handset_signalling.c \
    ..\..\av_headset_handset_signalling.h \
    ..\..\av_headset_hfp.c \
    ..\..\av_headset_hfp.h \
    ..\..\av_headset_init.c \
    ..\..\av_headset_init.h \
    ..\..\av_headset_ir.c \
    ..\..\av_headset_ir.h \
    ..\..\av_headset_kymera.c \
    ..\..\av_headset_kymera.h \
    ..\..\av_headset_kymera_a2dp.c \
    ..\..\av_headset_kymera_aec_leakthrough.c \
    ..\..\av_headset_kymera_anc.c \
    ..\..\av_headset_kymera_common.c \
    ..\..\av_headset_kymera_private.h \
    ..\..\av_headset_kymera_sco.c \
    ..\..\av_headset_kymera_sco_fwd.c \
    ..\..\av_headset_kymera_tones_prompts.c \
    ..\..\av_headset_latency.h \
    ..\..\av_headset_led.c \
    ..\..\av_headset_led.h \
    ..\..\av_headset_link_policy.c \
    ..\..\av_headset_link_policy.h \
    ..\..\av_headset_log.h \
    ..\..\av_headset_message.h \
    ..\..\av_headset_pairing.c \
    ..\..\av_headset_pairing.h \
    ..\..\av_headset_peer_signalling.c \
    ..\..\av_headset_peer_signalling.h \
    ..\..\av_headset_peer_sync.c \
    ..\..\av_headset_peer_sync.h \
    ..\..\av_headset_phy_state.c \
    ..\..\av_headset_phy_state.h \
    ..\..\av_headset_power.c \
    ..\..\av_headset_power.h \
    ..\..\av_headset_proximity.h \
    ..\..\av_headset_role.h \
    ..\..\av_headset_scan_manager.c \
    ..\..\av_headset_scan_manager.h \
    ..\..\av_headset_scofwd.c \
    ..\..\av_headset_scofwd.h \
    ..\..\av_headset_sdp.c \
    ..\..\av_headset_sdp.h \
    ..\..\av_headset_sm.c \
    ..\..\av_headset_sm.h \
    ..\..\av_headset_sm_private.h \
    ..\..\av_headset_stubs.c \
    ..\..\av_headset_tasklist.c \
    ..\..\av_headset_tasklist.h \
    ..\..\av_headset_temperature.c \
    ..\..\av_headset_temperature.h \
    ..\..\av_headset_temperature_sensor.h \
    ..\..\av_headset_test.c \
    ..\..\av_headset_test.h \
    ..\..\av_headset_test_le.c \
    ..\..\av_headset_test_le.h \
    ..\..\av_headset_ui.c \
    ..\..\av_headset_ui.h \
    ..\..\av_headset_upgrade.c \
    ..\..\av_headset_upgrade.h \
    ..\..\chains\chain_aac_stereo_decoder_left.chain \
    ..\..\chains\chain_aac_stereo_decoder_right.chain \
    ..\..\chains\chain_aptx_ad_tws_plus_decoder.chain \
    ..\..\chains\chain_aptx_mono_no_autosync_decoder.chain \
    ..\..\chains\chain_forwarding_input_aac_left.chain \
    ..\..\chains\chain_forwarding_input_aac_right.chain \
    ..\..\chains\chain_forwarding_input_aac_stereo_left.chain \
    ..\..\chains\chain_forwarding_input_aac_stereo_right.chain \
    ..\..\chains\chain_forwarding_input_aptx_left.chain \
    ..\..\chains\chain_forwarding_input_aptx_right.chain \
    ..\..\chains\chain_forwarding_input_sbc_left.chain \
    ..\..\chains\chain_forwarding_input_sbc_right.chain \
    ..\..\chains\chain_leakthrough_aec.chain \
    ..\..\chains\chain_micfwd_nb.chain \
    ..\..\chains\chain_micfwd_nb_2mic.chain \
    ..\..\chains\chain_micfwd_send.chain \
    ..\..\chains\chain_micfwd_send_2mic.chain \
    ..\..\chains\chain_micfwd_wb.chain \
    ..\..\chains\chain_micfwd_wb_2mic.chain \
    ..\..\chains\chain_output_volume.chain \
    ..\..\chains\chain_prompt_decoder.chain \
    ..\..\chains\chain_prompt_decoder_no_iir.chain \
    ..\..\chains\chain_prompt_pcm.chain \
    ..\..\chains\chain_sbc_mono_no_autosync_decoder.chain \
    ..\..\chains\chain_sco_nb.chain \
    ..\..\chains\chain_sco_nb_2mic.chain \
    ..\..\chains\chain_sco_swb.chain \
    ..\..\chains\chain_sco_wb.chain \
    ..\..\chains\chain_sco_wb_2mic.chain \
    ..\..\chains\chain_scofwd_nb.chain \
    ..\..\chains\chain_scofwd_nb_2mic.chain \
    ..\..\chains\chain_scofwd_recv.chain \
    ..\..\chains\chain_scofwd_recv_2mic.chain \
    ..\..\chains\chain_scofwd_wb.chain \
    ..\..\chains\chain_scofwd_wb_2mic.chain \
    ..\..\chains\chain_tone_gen.chain \
    ..\..\chains\chain_tone_gen_no_iir.chain \
    ..\..\main.c \
    ..\..\peripherals\adxl362.c \
    ..\..\peripherals\adxl362.h \
    ..\..\peripherals\thermistor.c \
    ..\..\peripherals\thermistor.h \
    ..\..\peripherals\thermistor_lp4549652p3m.h \
    ..\..\peripherals\thermistor_ncp15xh103.h \
    ..\..\peripherals\vncl3020.c \
    ..\..\peripherals\vncl3020.h \
    ..\common\1_button.pio.buttonxml \
    build_id_str.c \
$(DBS)

-include earbud.mak
# Check required variables have been defined
ifdef MAKEFILE_RULES_DIR
  $(info Using $(MAKEFILE_RULES_DIR)/Makefile.rules)
  include $(MAKEFILE_RULES_DIR)/Makefile.rules
else
  ifdef SDK
    include $(SDK)/Makefile.rules
  else
    $(error Variable SDK has not been defined. It should be set to the location of the Devkit tools folder.)
  endif
endif
