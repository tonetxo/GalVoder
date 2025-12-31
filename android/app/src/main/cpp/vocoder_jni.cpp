#include "VocoderEngine.h"
#include <jni.h>
#include <string>

extern "C" {

JNIEXPORT jlong JNICALL
Java_com_antigravity_vocodergal_audio_VocoderBridge_nativeCreate(JNIEnv *env,
                                                                 jobject thiz) {
  return reinterpret_cast<jlong>(new VocoderEngine());
}

JNIEXPORT void JNICALL
Java_com_antigravity_vocodergal_audio_VocoderBridge_nativeDestroy(
    JNIEnv *env, jobject thiz, jlong handle) {
  delete reinterpret_cast<VocoderEngine *>(handle);
}

JNIEXPORT jboolean JNICALL
Java_com_antigravity_vocodergal_audio_VocoderBridge_nativeStart(JNIEnv *env,
                                                                jobject thiz,
                                                                jlong handle) {
  auto *engine = reinterpret_cast<VocoderEngine *>(handle);
  return engine->start();
}

JNIEXPORT void JNICALL
Java_com_antigravity_vocodergal_audio_VocoderBridge_nativeStop(JNIEnv *env,
                                                               jobject thiz,
                                                               jlong handle) {
  auto *engine = reinterpret_cast<VocoderEngine *>(handle);
  engine->stop();
}

JNIEXPORT void JNICALL
Java_com_antigravity_vocodergal_audio_VocoderBridge_nativeSetPitch(
    JNIEnv *env, jobject thiz, jlong handle, jfloat pitch) {
  auto *engine = reinterpret_cast<VocoderEngine *>(handle);
  engine->setPitch(pitch);
}

JNIEXPORT void JNICALL
Java_com_antigravity_vocodergal_audio_VocoderBridge_nativeSetIntensity(
    JNIEnv *env, jobject thiz, jlong handle, jfloat intensity) {
  auto *engine = reinterpret_cast<VocoderEngine *>(handle);
  engine->setIntensity(intensity);
}

JNIEXPORT void JNICALL
Java_com_antigravity_vocodergal_audio_VocoderBridge_nativeSetWaveform(
    JNIEnv *env, jobject thiz, jlong handle, jint type) {
  auto *engine = reinterpret_cast<VocoderEngine *>(handle);
  engine->setWaveform(type);
}

JNIEXPORT void JNICALL
Java_com_antigravity_vocodergal_audio_VocoderBridge_nativeSetVibrato(
    JNIEnv *env, jobject thiz, jlong handle, jfloat amount) {
  auto *engine = reinterpret_cast<VocoderEngine *>(handle);
  engine->setVibrato(amount);
}

JNIEXPORT void JNICALL
Java_com_antigravity_vocodergal_audio_VocoderBridge_nativeSetEcho(
    JNIEnv *env, jobject thiz, jlong handle, jfloat amount) {
  auto *engine = reinterpret_cast<VocoderEngine *>(handle);
  engine->setEcho(amount);
}

JNIEXPORT jfloat JNICALL
Java_com_antigravity_vocodergal_audio_VocoderBridge_nativeGetVULevel(
    JNIEnv *env, jobject thiz, jlong handle) {
  auto *engine = reinterpret_cast<VocoderEngine *>(handle);
  return engine->getVULevel();
}

JNIEXPORT jfloatArray JNICALL
Java_com_antigravity_vocodergal_audio_VocoderBridge_nativeGetWaveformData(
    JNIEnv *env, jobject thiz, jlong handle) {
  auto *engine = reinterpret_cast<VocoderEngine *>(handle);
  auto waveform = engine->getWaveformData();

  jfloatArray result = env->NewFloatArray(waveform.size());
  env->SetFloatArrayRegion(result, 0, waveform.size(), waveform.data());
  return result;
}

} // extern "C"
