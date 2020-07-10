/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*********************************************************************************/

/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: speech.proto */

#ifndef PROTOBUF_C_speech_2eproto__INCLUDED
#define PROTOBUF_C_speech_2eproto__INCLUDED

#include <protobuf.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif

#include "common.pb-c.h"

typedef struct _Dialog Dialog;
typedef struct _SpeechSettings SpeechSettings;
typedef struct _SpeechInitiator SpeechInitiator;
typedef struct _SpeechInitiator__WakeWord SpeechInitiator__WakeWord;
typedef struct _StartSpeech StartSpeech;
typedef struct _SpeechProvider SpeechProvider;
typedef struct _ProvideSpeech ProvideSpeech;
typedef struct _StopSpeech StopSpeech;
typedef struct _EndpointSpeech EndpointSpeech;
typedef struct _NotifySpeechState NotifySpeechState;


/* --- enums --- */

typedef enum _SpeechInitiator__Type {
  SPEECH_INITIATOR__TYPE__NONE = 0,
  SPEECH_INITIATOR__TYPE__PRESS_AND_HOLD = 1,
  SPEECH_INITIATOR__TYPE__TAP = 3,
  SPEECH_INITIATOR__TYPE__WAKEWORD = 4
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(SPEECH_INITIATOR__TYPE)
} SpeechInitiator__Type;
typedef enum _AudioProfile {
  AUDIO_PROFILE__CLOSE_TALK = 0,
  AUDIO_PROFILE__NEAR_FIELD = 1,
  AUDIO_PROFILE__FAR_FIELD = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(AUDIO_PROFILE)
} AudioProfile;
typedef enum _AudioFormat {
  AUDIO_FORMAT__PCM_L16_16KHZ_MONO = 0,
  AUDIO_FORMAT__OPUS_16KHZ_32KBPS_CBR_0_20MS = 1,
  AUDIO_FORMAT__OPUS_16KHZ_16KBPS_CBR_0_20MS = 2,
  AUDIO_FORMAT__MSBC = 3
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(AUDIO_FORMAT)
} AudioFormat;
typedef enum _AudioSource {
  AUDIO_SOURCE__STREAM = 0,
  AUDIO_SOURCE__BLUETOOTH_SCO = 1
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(AUDIO_SOURCE)
} AudioSource;
typedef enum _SpeechState {
  SPEECH_STATE__IDLE = 0,
  SPEECH_STATE__LISTENING = 1,
  SPEECH_STATE__PROCESSING = 2,
  SPEECH_STATE__SPEAKING = 3
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(SPEECH_STATE)
} SpeechState;

/* --- messages --- */

struct  _Dialog
{
  ProtobufCMessage base;
  uint32_t id;
};
#define DIALOG__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&dialog__descriptor) \
    , 0 }


struct  _SpeechSettings
{
  ProtobufCMessage base;
  AudioProfile audio_profile;
  AudioFormat audio_format;
  AudioSource audio_source;
};
#define SPEECH_SETTINGS__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&speech_settings__descriptor) \
    , AUDIO_PROFILE__CLOSE_TALK, AUDIO_FORMAT__PCM_L16_16KHZ_MONO, AUDIO_SOURCE__STREAM }


struct  _SpeechInitiator__WakeWord
{
  ProtobufCMessage base;
  uint32_t start_index_in_samples;
  uint32_t end_index_in_samples;
  protobuf_c_boolean near_miss;
  ProtobufCBinaryData metadata;
};
#define SPEECH_INITIATOR__WAKE_WORD__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&speech_initiator__wake_word__descriptor) \
    , 0, 0, 0, {0,NULL} }


struct  _SpeechInitiator
{
  ProtobufCMessage base;
  SpeechInitiator__Type type;
  SpeechInitiator__WakeWord *wake_word;
};
#define SPEECH_INITIATOR__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&speech_initiator__descriptor) \
    , SPEECH_INITIATOR__TYPE__NONE, NULL }


struct  _StartSpeech
{
  ProtobufCMessage base;
  SpeechSettings *settings;
  SpeechInitiator *initiator;
  Dialog *dialog;
  protobuf_c_boolean suppressearcon;
};
#define START_SPEECH__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&start_speech__descriptor) \
    , NULL, NULL, NULL, 0 }


struct  _SpeechProvider
{
  ProtobufCMessage base;
  SpeechSettings *speech_settings;
  Dialog *dialog;
};
#define SPEECH_PROVIDER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&speech_provider__descriptor) \
    , NULL, NULL }


struct  _ProvideSpeech
{
  ProtobufCMessage base;
  Dialog *dialog;
};
#define PROVIDE_SPEECH__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&provide_speech__descriptor) \
    , NULL }


struct  _StopSpeech
{
  ProtobufCMessage base;
  ErrorCode error_code;
  Dialog *dialog;
};
#define STOP_SPEECH__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&stop_speech__descriptor) \
    , ERROR_CODE__SUCCESS, NULL }


struct  _EndpointSpeech
{
  ProtobufCMessage base;
  Dialog *dialog;
};
#define ENDPOINT_SPEECH__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&endpoint_speech__descriptor) \
    , NULL }


struct  _NotifySpeechState
{
  ProtobufCMessage base;
  SpeechState state;
};
#define NOTIFY_SPEECH_STATE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&notify_speech_state__descriptor) \
    , SPEECH_STATE__IDLE }


/* Dialog methods */
void   dialog__init
                     (Dialog         *message);
size_t dialog__get_packed_size
                     (const Dialog   *message);
size_t dialog__pack
                     (const Dialog   *message,
                      uint8_t             *out);
size_t dialog__pack_to_buffer
                     (const Dialog   *message,
                      ProtobufCBuffer     *buffer);
Dialog *
       dialog__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   dialog__free_unpacked
                     (Dialog *message,
                      ProtobufCAllocator *allocator);
/* SpeechSettings methods */
void   speech_settings__init
                     (SpeechSettings         *message);
size_t speech_settings__get_packed_size
                     (const SpeechSettings   *message);
size_t speech_settings__pack
                     (const SpeechSettings   *message,
                      uint8_t             *out);
size_t speech_settings__pack_to_buffer
                     (const SpeechSettings   *message,
                      ProtobufCBuffer     *buffer);
SpeechSettings *
       speech_settings__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   speech_settings__free_unpacked
                     (SpeechSettings *message,
                      ProtobufCAllocator *allocator);
/* SpeechInitiator__WakeWord methods */
void   speech_initiator__wake_word__init
                     (SpeechInitiator__WakeWord         *message);
/* SpeechInitiator methods */
void   speech_initiator__init
                     (SpeechInitiator         *message);
size_t speech_initiator__get_packed_size
                     (const SpeechInitiator   *message);
size_t speech_initiator__pack
                     (const SpeechInitiator   *message,
                      uint8_t             *out);
size_t speech_initiator__pack_to_buffer
                     (const SpeechInitiator   *message,
                      ProtobufCBuffer     *buffer);
SpeechInitiator *
       speech_initiator__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   speech_initiator__free_unpacked
                     (SpeechInitiator *message,
                      ProtobufCAllocator *allocator);
/* StartSpeech methods */
void   start_speech__init
                     (StartSpeech         *message);
size_t start_speech__get_packed_size
                     (const StartSpeech   *message);
size_t start_speech__pack
                     (const StartSpeech   *message,
                      uint8_t             *out);
size_t start_speech__pack_to_buffer
                     (const StartSpeech   *message,
                      ProtobufCBuffer     *buffer);
StartSpeech *
       start_speech__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   start_speech__free_unpacked
                     (StartSpeech *message,
                      ProtobufCAllocator *allocator);
/* SpeechProvider methods */
void   speech_provider__init
                     (SpeechProvider         *message);
size_t speech_provider__get_packed_size
                     (const SpeechProvider   *message);
size_t speech_provider__pack
                     (const SpeechProvider   *message,
                      uint8_t             *out);
size_t speech_provider__pack_to_buffer
                     (const SpeechProvider   *message,
                      ProtobufCBuffer     *buffer);
SpeechProvider *
       speech_provider__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   speech_provider__free_unpacked
                     (SpeechProvider *message,
                      ProtobufCAllocator *allocator);
/* ProvideSpeech methods */
void   provide_speech__init
                     (ProvideSpeech         *message);
size_t provide_speech__get_packed_size
                     (const ProvideSpeech   *message);
size_t provide_speech__pack
                     (const ProvideSpeech   *message,
                      uint8_t             *out);
size_t provide_speech__pack_to_buffer
                     (const ProvideSpeech   *message,
                      ProtobufCBuffer     *buffer);
ProvideSpeech *
       provide_speech__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   provide_speech__free_unpacked
                     (ProvideSpeech *message,
                      ProtobufCAllocator *allocator);
/* StopSpeech methods */
void   stop_speech__init
                     (StopSpeech         *message);
size_t stop_speech__get_packed_size
                     (const StopSpeech   *message);
size_t stop_speech__pack
                     (const StopSpeech   *message,
                      uint8_t             *out);
size_t stop_speech__pack_to_buffer
                     (const StopSpeech   *message,
                      ProtobufCBuffer     *buffer);
StopSpeech *
       stop_speech__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   stop_speech__free_unpacked
                     (StopSpeech *message,
                      ProtobufCAllocator *allocator);
/* EndpointSpeech methods */
void   endpoint_speech__init
                     (EndpointSpeech         *message);
size_t endpoint_speech__get_packed_size
                     (const EndpointSpeech   *message);
size_t endpoint_speech__pack
                     (const EndpointSpeech   *message,
                      uint8_t             *out);
size_t endpoint_speech__pack_to_buffer
                     (const EndpointSpeech   *message,
                      ProtobufCBuffer     *buffer);
EndpointSpeech *
       endpoint_speech__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   endpoint_speech__free_unpacked
                     (EndpointSpeech *message,
                      ProtobufCAllocator *allocator);
/* NotifySpeechState methods */
void   notify_speech_state__init
                     (NotifySpeechState         *message);
size_t notify_speech_state__get_packed_size
                     (const NotifySpeechState   *message);
size_t notify_speech_state__pack
                     (const NotifySpeechState   *message,
                      uint8_t             *out);
size_t notify_speech_state__pack_to_buffer
                     (const NotifySpeechState   *message,
                      ProtobufCBuffer     *buffer);
NotifySpeechState *
       notify_speech_state__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   notify_speech_state__free_unpacked
                     (NotifySpeechState *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Dialog_Closure)
                 (const Dialog *message,
                  void *closure_data);
typedef void (*SpeechSettings_Closure)
                 (const SpeechSettings *message,
                  void *closure_data);
typedef void (*SpeechInitiator__WakeWord_Closure)
                 (const SpeechInitiator__WakeWord *message,
                  void *closure_data);
typedef void (*SpeechInitiator_Closure)
                 (const SpeechInitiator *message,
                  void *closure_data);
typedef void (*StartSpeech_Closure)
                 (const StartSpeech *message,
                  void *closure_data);
typedef void (*SpeechProvider_Closure)
                 (const SpeechProvider *message,
                  void *closure_data);
typedef void (*ProvideSpeech_Closure)
                 (const ProvideSpeech *message,
                  void *closure_data);
typedef void (*StopSpeech_Closure)
                 (const StopSpeech *message,
                  void *closure_data);
typedef void (*EndpointSpeech_Closure)
                 (const EndpointSpeech *message,
                  void *closure_data);
typedef void (*NotifySpeechState_Closure)
                 (const NotifySpeechState *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCEnumDescriptor    audio_profile__descriptor;
extern const ProtobufCEnumDescriptor    audio_format__descriptor;
extern const ProtobufCEnumDescriptor    audio_source__descriptor;
extern const ProtobufCEnumDescriptor    speech_state__descriptor;
extern const ProtobufCMessageDescriptor dialog__descriptor;
extern const ProtobufCMessageDescriptor speech_settings__descriptor;
extern const ProtobufCMessageDescriptor speech_initiator__descriptor;
extern const ProtobufCMessageDescriptor speech_initiator__wake_word__descriptor;
extern const ProtobufCEnumDescriptor    speech_initiator__type__descriptor;
extern const ProtobufCMessageDescriptor start_speech__descriptor;
extern const ProtobufCMessageDescriptor speech_provider__descriptor;
extern const ProtobufCMessageDescriptor provide_speech__descriptor;
extern const ProtobufCMessageDescriptor stop_speech__descriptor;
extern const ProtobufCMessageDescriptor endpoint_speech__descriptor;
extern const ProtobufCMessageDescriptor notify_speech_state__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_speech_2eproto__INCLUDED */
