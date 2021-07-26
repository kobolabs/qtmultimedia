/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include <qaudio.h>
#include <qmath.h>
#include <QDebug>

QT_BEGIN_NAMESPACE

static void qRegisterAudioMetaTypes()
{
    qRegisterMetaType<QAudio::Error>();
    qRegisterMetaType<QAudio::State>();
    qRegisterMetaType<QAudio::Mode>();
    qRegisterMetaType<QAudio::Role>();
    qRegisterMetaType<QAudio::VolumeScale>();
}

Q_CONSTRUCTOR_FUNCTION(qRegisterAudioMetaTypes)

/*!
    \namespace QAudio
    \ingroup multimedia-namespaces
    \brief The QAudio namespace contains enums used by the audio classes.
    \inmodule QtMultimedia
    \ingroup multimedia
    \ingroup multimedia_audio
*/

/*!
    \enum QAudio::Error

    \value NoError         No errors have occurred
    \value OpenError       An error occurred opening the audio device
    \value IOError         An error occurred during read/write of audio device
    \value UnderrunError   Audio data is not being fed to the audio device at a fast enough rate
    \value FatalError      A non-recoverable error has occurred, the audio device is not usable at this time.
*/

/*!
    \enum QAudio::State

    \value ActiveState     Audio data is being processed, this state is set after start() is called
                           and while audio data is available to be processed.
    \value SuspendedState  The audio device is in a suspended state, this state will only be entered
                           after suspend() is called.
    \value StoppedState    The audio device is closed, and is not processing any audio data
    \value IdleState       The QIODevice passed in has no data and audio system's buffer is empty, this state
                           is set after start() is called and while no audio data is available to be processed.
*/

/*!
    \enum QAudio::Mode

    \value AudioOutput   audio output device
    \value AudioInput    audio input device
*/

/*!
    \enum QAudio::Role

    This enum describes the role of an audio stream.

    \value UnknownRole              The role is unknown or undefined
    \value MusicRole                Music
    \value VideoRole                Soundtrack from a movie or a video
    \value VoiceCommunicationRole   Voice communications, such as telephony
    \value AlarmRole                Alarm
    \value NotificationRole         Notification, such as an incoming e-mail or a chat request
    \value RingtoneRole             Ringtone
    \value AccessibilityRole        For accessibility, such as with a screen reader
    \value SonificationRole         Sonification, such as with user interface sounds
    \value GameRole                 Game audio

    \since 5.6
    \sa QMediaPlayer::setAudioRole()
*/

/*!
    \enum QAudio::VolumeScale

    This enum defines the different audio volume scales.

    \value LinearVolumeScale     Linear scale. \c 0.0 (0%) is silence and \c 1.0 (100%) is full
                                 volume. All Qt Multimedia classes that have an audio volume use a
                                 linear scale.
    \value CubicVolumeScale      Cubic scale. \c 0.0 (0%) is silence and \c 1.0 (100%) is full
                                 volume. UI volume controls should usually use a cubic scale.
    \value DecibelVolumeScale    Decibel (dB, amplitude) logarithmic scale. \c -200 is silence and
                                 \c 0 is full volume.

    \since 5.8
    \sa QAudio::convertVolume()
*/

namespace QAudio
{

/*!
    \fn qreal QAudio::convertVolume(qreal volume, VolumeScale from, VolumeScale to)

    Converts an audio \a volume \a from a volume scale \a to another, and returns the result.

    Depending on the context, different scales are used to represent audio volume. All Qt Multimedia
    classes that have an audio volume use a linear scale, the reason is that the loudness of a
    speaker is controlled by modulating its voltage on a linear scale. The human ear on the other
    hand, perceives loudness in a logarithmic way. That is why the decibel scale, being a logarithmic
    scale, is typically used to define sound levels. UI volume controls in professional audio
    applications usually use a decibel scale. The cubic scale is a computationally cheap
    approximation of a logarithmic scale, most applications should use a cubic scale for their UI
    volume controls.

    The following example shows how to convert the volume value from a slider control before passing
    it to a QMediaPlayer. As a result, the perceived increase in volume is the same when increasing
    the volume slider from 20 to 30 as it is from 50 to 60:

    \snippet multimedia-snippets/audio.cpp Volume conversion

    \since 5.8
    \sa VolumeScale, QMediaPlayer::setVolume(), QAudioOutput::setVolume(),
        QAudioInput::setVolume(), QSoundEffect::setVolume(), QMediaRecorder::setVolume()
*/
qreal convertVolume(qreal volume, VolumeScale from, VolumeScale to)
{
    switch (from) {
    case LinearVolumeScale:
        volume = qMax(qreal(0), volume);
        switch (to) {
        case LinearVolumeScale:
            return volume;
        case CubicVolumeScale:
            return qPow(volume, qreal(1 / 3.0));
        case DecibelVolumeScale:
            if (volume < 0.001)
                return qreal(-200);
            else
                return qreal(20.0) * std::log10(volume);
        }
        break;
    case CubicVolumeScale:
        volume = qMax(qreal(0), volume);
        switch (to) {
        case LinearVolumeScale:
            return volume * volume * volume;
        case CubicVolumeScale:
            return volume;
        case DecibelVolumeScale:
            if (volume < 0.001)
                return qreal(-200);
            else
                return qreal(3.0 * 20.0) * std::log10(volume);
        }
        break;
    case DecibelVolumeScale:
        switch (to) {
        case LinearVolumeScale:
            return qPow(qreal(10.0), volume / qreal(20.0));
        case CubicVolumeScale:
            return qPow(qreal(10.0), volume / qreal(3.0 * 20.0));
        case DecibelVolumeScale:
            return volume;
        }
        break;
    }

    return volume;
}

}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, QAudio::Error error)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    switch (error) {
        case QAudio::NoError:
            dbg << "NoError";
            break;
        case QAudio::OpenError:
            dbg << "OpenError";
            break;
        case QAudio::IOError:
            dbg << "IOError";
            break;
        case QAudio::UnderrunError:
            dbg << "UnderrunError";
            break;
        case QAudio::FatalError:
            dbg << "FatalError";
            break;
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, QAudio::State state)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    switch (state) {
        case QAudio::ActiveState:
            dbg << "ActiveState";
            break;
        case QAudio::SuspendedState:
            dbg << "SuspendedState";
            break;
        case QAudio::StoppedState:
            dbg << "StoppedState";
            break;
        case QAudio::IdleState:
            dbg << "IdleState";
            break;
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, QAudio::Mode mode)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    switch (mode) {
        case QAudio::AudioInput:
            dbg << "AudioInput";
            break;
        case QAudio::AudioOutput:
            dbg << "AudioOutput";
            break;
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, QAudio::Role role)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    switch (role) {
    case QAudio::UnknownRole:
        dbg << "UnknownRole";
        break;
    case QAudio::AccessibilityRole:
        dbg << "AccessibilityRole";
        break;
    case QAudio::AlarmRole:
        dbg << "AlarmRole";
        break;
    case QAudio::GameRole:
        dbg << "GameRole";
        break;
    case QAudio::MusicRole:
        dbg << "MusicRole";
        break;
    case QAudio::NotificationRole:
        dbg << "NotificationRole";
        break;
    case QAudio::RingtoneRole:
        dbg << "RingtoneRole";
        break;
    case QAudio::SonificationRole:
        dbg << "SonificationRole";
        break;
    case QAudio::VideoRole:
        dbg << "VideoRole";
        break;
    case QAudio::VoiceCommunicationRole:
        dbg << "VoiceCommunicationRole";
        break;
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, QAudio::VolumeScale scale)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    switch (scale) {
    case QAudio::LinearVolumeScale:
        dbg << "LinearVolumeScale";
        break;
    case QAudio::CubicVolumeScale:
        dbg << "CubicVolumeScale";
        break;
    case QAudio::DecibelVolumeScale:
        dbg << "DecibelVolumeScale";
        break;
    }
    return dbg;
}

#endif


QT_END_NAMESPACE

