#include "RetroAudio.h"
#include <QMediaDevices>
#include <QAudioDevice>
#include <cmath>
#include <numbers>
#include <algorithm>

RetroAudio::RetroAudio(QObject* parent)
    : QObject(parent)
{
    initAudio();
}

RetroAudio::~RetroAudio() {
    if (m_sink) {
        m_sink->stop();
    }
    if (m_bgmSink) {
        m_bgmSink->stop();
    }
}

void RetroAudio::initAudio() {
    constexpr int sampleRate = 44100;
    m_format.setSampleRate(sampleRate);
    m_format.setChannelCount(1);
    m_format.setSampleFormat(QAudioFormat::Int16);

    QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (device.isNull()) {
        m_initialized = false;
        return;
    }

    // 1. One-shot SFX synthesis
    m_jumpSfx = generateSweep(sampleRate, 260.0, 580.0, 0.08, 0.25);
    
    QByteArray tone1 = generateTone(sampleRate, 880.0, 0.08, 0.25);
    QByteArray silence = QByteArray(static_cast<qsizetype>(sampleRate * sizeof(int16_t) * 0.02), 0);
    QByteArray tone2 = generateTone(sampleRate, 1175.0, 0.12, 0.25);
    m_scoreSfx = tone1 + silence + tone2;

    m_deathSfx = generateSweep(sampleRate, 240.0, 70.0, 0.28, 0.35);

    // PowerUp: C5 -> E5 -> G5 upward arpeggio
    QByteArray p1 = generateTone(sampleRate, 523.25, 0.06, 0.22);
    QByteArray p2 = generateTone(sampleRate, 659.25, 0.06, 0.22);
    QByteArray p3 = generateTone(sampleRate, 783.99, 0.14, 0.25);
    m_powerupSfx = p1 + p2 + p3;

    // Shield break: sharp metallic crunch
    m_shieldBreakSfx = generateSweep(sampleRate, 880.0, 140.0, 0.18, 0.35);

    // Thunder: low rumbling sweep
    m_thunderSfx = generateSweep(sampleRate, 95.0, 32.0, 0.45, 0.38);

    // 2. Dedicated SFX audio sink
    m_sink = std::make_unique<QAudioSink>(device, m_format, this);
    m_buffer = std::make_unique<QBuffer>(this);

    // 3. Dedicated Looping Chiptune BGM audio sink
    m_bgmData = generateChiptuneTrack(sampleRate);
    m_bgmSink = std::make_unique<QAudioSink>(device, m_format, this);
    m_bgmSink->setVolume(0.22);
    m_bgmBuffer = std::make_unique<QBuffer>(this);

    connect(m_bgmSink.get(), &QAudioSink::stateChanged, this, [this](QAudio::State state) {
        if (state == QAudio::IdleState && m_bgmActive && !m_muted && m_bgmBuffer) {
            m_bgmBuffer->seek(0);
            m_bgmSink->start(m_bgmBuffer.get());
        }
    });

    m_initialized = true;
}

QByteArray RetroAudio::generateTone(int sampleRate, double freq, double durationSec, double volume) {
    int totalSamples = static_cast<int>(sampleRate * durationSec);
    QByteArray pcm;
    pcm.resize(totalSamples * sizeof(int16_t));
    int16_t* ptr = reinterpret_cast<int16_t*>(pcm.data());

    for (int i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double phase = std::fmod(t * freq, 1.0);
        double sample = (phase < 0.5) ? 1.0 : -1.0;
        double envelope = 1.0 - (static_cast<double>(i) / totalSamples);
        ptr[i] = static_cast<int16_t>(sample * envelope * volume * 32767.0);
    }
    return pcm;
}

QByteArray RetroAudio::generateSweep(int sampleRate, double freqStart, double freqEnd, double durationSec, double volume) {
    int totalSamples = static_cast<int>(sampleRate * durationSec);
    QByteArray pcm;
    pcm.resize(totalSamples * sizeof(int16_t));
    int16_t* ptr = reinterpret_cast<int16_t*>(pcm.data());

    double phase = 0.0;
    for (int i = 0; i < totalSamples; ++i) {
        double progress = static_cast<double>(i) / totalSamples;
        double currentFreq = freqStart + (freqEnd - freqStart) * progress;
        phase += 2.0 * std::numbers::pi * currentFreq / sampleRate;
        if (phase > 2.0 * std::numbers::pi) {
            phase -= 2.0 * std::numbers::pi;
        }

        double sample = (std::sin(phase) > 0.0) ? 0.8 : -0.8;
        double envelope = 1.0 - progress * 0.7;
        ptr[i] = static_cast<int16_t>(sample * envelope * volume * 32767.0);
    }
    return pcm;
}

QByteArray RetroAudio::generateChiptuneTrack(int sampleRate) {
    constexpr double stepSec = 0.120;
    constexpr int totalSteps = 32;
    int totalSamples = static_cast<int>(sampleRate * stepSec * totalSteps);

    QByteArray pcm;
    pcm.resize(totalSamples * sizeof(int16_t));
    int16_t* ptr = reinterpret_cast<int16_t*>(pcm.data());

    // 32-step 8-bit lead melody
    static const double melody[32] = {
        440.0,   0.0, 440.0, 523.25, 493.88,   0.0, 392.0, 440.0,
        523.25, 587.33, 659.25,   0.0, 587.33, 523.25, 493.88,   0.0,
        440.0,   0.0, 392.0, 440.0, 523.25,   0.0, 659.25, 587.33,
        523.25, 493.88, 440.0, 392.0, 440.0,   0.0,   0.0,   0.0
    };

    // 32-step walking bassline
    static const double bass[32] = {
        110.0, 110.0, 164.81, 164.81,  98.0,  98.0, 146.83, 146.83,
         87.31,  87.31, 130.81, 130.81,  82.41,  82.41, 123.47, 123.47,
        110.0, 110.0, 164.81, 164.81,  98.0,  98.0, 146.83, 146.83,
         87.31,  87.31, 130.81, 130.81, 110.0, 110.0, 110.0, 110.0
    };

    double leadPhase = 0.0;
    double bassPhase = 0.0;
    uint32_t noiseState = 0x12345678;

    for (int i = 0; i < totalSamples; ++i) {
        int step = static_cast<int>((static_cast<double>(i) / sampleRate) / stepSec) % 32;
        double stepFrac = std::fmod((static_cast<double>(i) / sampleRate), stepSec) / stepSec;

        // 1. Lead Melody Voice (Square wave 50% duty cycle)
        double leadFreq = melody[step];
        double leadSample = 0.0;
        if (leadFreq > 10.0) {
            leadPhase += 2.0 * std::numbers::pi * leadFreq / sampleRate;
            if (leadPhase > 2.0 * std::numbers::pi) leadPhase -= 2.0 * std::numbers::pi;
            double envelope = std::exp(-stepFrac * 3.5);
            leadSample = (std::sin(leadPhase) >= 0.0 ? 1.0 : -1.0) * envelope;
        }

        // 2. Bass Voice (Square wave 25% duty cycle, deep punch)
        double bFreq = bass[step];
        bassPhase += 2.0 * std::numbers::pi * bFreq / sampleRate;
        if (bassPhase > 2.0 * std::numbers::pi) bassPhase -= 2.0 * std::numbers::pi;
        double bEnv = std::exp(-stepFrac * 2.0);
        double bassSample = (std::fmod(bassPhase / (2.0 * std::numbers::pi), 1.0) < 0.25 ? 1.0 : -1.0) * bEnv;

        // 3. Noise Percussion (Hi-hat / Snare on offbeats)
        double noiseSample = 0.0;
        if (step % 2 == 1 && stepFrac < 0.25) {
            noiseState = noiseState * 1664525u + 1013904223u;
            double n = ((noiseState >> 16) & 0x7FFF) / 16384.0 - 1.0;
            double nEnv = 1.0 - (stepFrac / 0.25);
            noiseSample = n * nEnv;
        }

        double mixed = leadSample * 0.40 + bassSample * 0.35 + noiseSample * 0.15;
        mixed = std::clamp(mixed, -1.0, 1.0);
        ptr[i] = static_cast<int16_t>(mixed * 26000.0);
    }

    return pcm;
}

void RetroAudio::startBgm() {
    if (!m_initialized || m_muted || !m_bgmSink || m_bgmData.isEmpty()) return;
    m_bgmSink->stop();
    m_bgmBuffer->close();
    m_bgmBuffer->setData(m_bgmData);
    m_bgmBuffer->open(QIODevice::ReadOnly);
    m_bgmSink->start(m_bgmBuffer.get());
}

void RetroAudio::stopBgm() {
    if (m_bgmSink) {
        m_bgmSink->stop();
    }
}

void RetroAudio::toggleBgm() {
    m_bgmActive = !m_bgmActive;
    if (m_bgmActive) {
        startBgm();
    } else {
        stopBgm();
    }
}

void RetroAudio::setMuted(bool mute) {
    m_muted = mute;
    if (m_muted) {
        if (m_sink) m_sink->stop();
        if (m_bgmSink) m_bgmSink->stop();
    } else {
        if (m_bgmActive) {
            startBgm();
        }
    }
}

void RetroAudio::playWave(const QByteArray& pcmData) {
    if (!m_initialized || m_muted || !m_sink) return;
    
    m_sink->stop();
    m_buffer->close();
    m_buffer->setData(pcmData);
    m_buffer->open(QIODevice::ReadOnly);
    m_sink->start(m_buffer.get());
}

void RetroAudio::playJump() {
    playWave(m_jumpSfx);
}

void RetroAudio::playScoreMilestone() {
    playWave(m_scoreSfx);
}

void RetroAudio::playGameOver() {
    playWave(m_deathSfx);
}

void RetroAudio::playPowerUp() {
    playWave(m_powerupSfx);
}

void RetroAudio::playShieldBreak() {
    playWave(m_shieldBreakSfx);
}

void RetroAudio::playThunder() {
    playWave(m_thunderSfx);
}
