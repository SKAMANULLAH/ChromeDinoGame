#ifndef RETROAUDIO_H
#define RETROAUDIO_H

#include <QObject>
#include <QByteArray>
#include <QAudioFormat>
#include <QAudioSink>
#include <QBuffer>
#include <memory>

/**
 * @file RetroAudio.h
 * @brief Procedural 8-bit Retro Sound Generator for Chrome Dino.
 * 
 * Synthesizes square waves and chirps in memory so the project requires
 * zero external sound assets.
 */
class RetroAudio : public QObject {
    Q_OBJECT
public:
    explicit RetroAudio(QObject* parent = nullptr);
    ~RetroAudio() override;

    void playJump();
    void playScoreMilestone();
    void playGameOver();
    void playPowerUp();
    void playShieldBreak();
    void playThunder();

    void setMuted(bool mute);
    bool isMuted() const { return m_muted; }

    void toggleBgm();
    bool isBgmActive() const { return m_bgmActive; }

private:
    void initAudio();
    void playWave(const QByteArray& pcmData);
    void startBgm();
    void stopBgm();

    bool m_muted{false};
    bool m_bgmActive{false};
    bool m_initialized{false};
    QAudioFormat m_format;

    // SFX Sink & Buffer
    std::unique_ptr<QAudioSink> m_sink;
    std::unique_ptr<QBuffer> m_buffer;

    // Independent Looping BGM Sink & Buffer
    std::unique_ptr<QAudioSink> m_bgmSink;
    std::unique_ptr<QBuffer> m_bgmBuffer;
    QByteArray m_bgmData;

    QByteArray m_jumpSfx;
    QByteArray m_scoreSfx;
    QByteArray m_deathSfx;
    QByteArray m_powerupSfx;
    QByteArray m_shieldBreakSfx;
    QByteArray m_thunderSfx;

    static QByteArray generateSweep(int sampleRate, double freqStart, double freqEnd, double durationSec, double volume = 0.3);
    static QByteArray generateTone(int sampleRate, double freq, double durationSec, double volume = 0.3);
    static QByteArray generateChiptuneTrack(int sampleRate);
};

#endif // RETROAUDIO_H
