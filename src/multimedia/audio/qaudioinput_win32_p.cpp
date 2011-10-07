/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// INTERNAL USE ONLY: Do NOT use for any other purpose.
//


#include "qaudioinput_win32_p.h"

QT_BEGIN_NAMESPACE

//#define DEBUG_AUDIO 1

QAudioInputPrivate::QAudioInputPrivate(const QByteArray &device)
{
    bytesAvailable = 0;
    buffer_size = 0;
    period_size = 0;
    m_device = device;
    totalTimeValue = 0;
    intervalTime = 1000;
    errorState = QAudio::NoError;
    deviceState = QAudio::StoppedState;
    audioSource = 0;
    pullMode = true;
    resuming = false;
    finished = false;
    waveBlockOffset = 0;
}

QAudioInputPrivate::~QAudioInputPrivate()
{
    stop();
}

void QT_WIN_CALLBACK QAudioInputPrivate::waveInProc( HWAVEIN hWaveIn, UINT uMsg,
        DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 )
{
    Q_UNUSED(dwParam1)
    Q_UNUSED(dwParam2)
    Q_UNUSED(hWaveIn)

    QAudioInputPrivate* qAudio;
    qAudio = (QAudioInputPrivate*)(dwInstance);
    if(!qAudio)
        return;

    QMutexLocker(&qAudio->mutex);

    switch(uMsg) {
        case WIM_OPEN:
            break;
        case WIM_DATA:
            if(qAudio->waveFreeBlockCount > 0)
                qAudio->waveFreeBlockCount--;
            qAudio->feedback();
            break;
        case WIM_CLOSE:
            qAudio->finished = true;
            break;
        default:
            return;
    }
}

WAVEHDR* QAudioInputPrivate::allocateBlocks(int size, int count)
{
    int i;
    unsigned char* buffer;
    WAVEHDR* blocks;
    DWORD totalBufferSize = (size + sizeof(WAVEHDR))*count;

    if((buffer=(unsigned char*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,
                    totalBufferSize)) == 0) {
        qWarning("QAudioInput: Memory allocation error");
        return 0;
    }
    blocks = (WAVEHDR*)buffer;
    buffer += sizeof(WAVEHDR)*count;
    for(i = 0; i < count; i++) {
        blocks[i].dwBufferLength = size;
        blocks[i].lpData = (LPSTR)buffer;
        blocks[i].dwBytesRecorded=0;
        blocks[i].dwUser = 0L;
        blocks[i].dwFlags = 0L;
        blocks[i].dwLoops = 0L;
        result = waveInPrepareHeader(hWaveIn,&blocks[i], sizeof(WAVEHDR));
        if(result != MMSYSERR_NOERROR) {
            qWarning("QAudioInput: Can't prepare block %d",i);
            return 0;
        }
        buffer += size;
    }
    return blocks;
}

void QAudioInputPrivate::freeBlocks(WAVEHDR* blockArray)
{
    WAVEHDR* blocks = blockArray;

    int count = buffer_size/period_size;

    for(int i = 0; i < count; i++) {
        waveInUnprepareHeader(hWaveIn,blocks, sizeof(WAVEHDR));
        blocks++;
    }
    HeapFree(GetProcessHeap(), 0, blockArray);
}

QAudio::Error QAudioInputPrivate::error() const
{
    return errorState;
}

QAudio::State QAudioInputPrivate::state() const
{
    return deviceState;
}

void QAudioInputPrivate::setFormat(const QAudioFormat& fmt)
{
    if (deviceState == QAudio::StoppedState)
        settings = fmt;
}

QAudioFormat QAudioInputPrivate::format() const
{
    return settings;
}

void QAudioInputPrivate::start(QIODevice* device)
{
    if(deviceState != QAudio::StoppedState)
        close();

    if(!pullMode && audioSource)
        delete audioSource;

    pullMode = true;
    audioSource = device;

    deviceState = QAudio::ActiveState;

    if(!open())
        return;

    emit stateChanged(deviceState);
}

QIODevice* QAudioInputPrivate::start()
{
    if(deviceState != QAudio::StoppedState)
        close();

    if(!pullMode && audioSource)
        delete audioSource;

    pullMode = false;
    audioSource = new InputPrivate(this);
    audioSource->open(QIODevice::ReadOnly | QIODevice::Unbuffered);

    deviceState = QAudio::IdleState;

    if(!open())
        return 0;

    emit stateChanged(deviceState);

    return audioSource;
}

void QAudioInputPrivate::stop()
{
    if(deviceState == QAudio::StoppedState)
        return;

    close();
    emit stateChanged(deviceState);
}

bool QAudioInputPrivate::open()
{
#ifdef DEBUG_AUDIO
    QTime now(QTime::currentTime());
    qDebug()<<now.second()<<"s "<<now.msec()<<"ms :open()";
#endif
    header = 0;

    period_size = 0;

    if (!settings.isValid()) {
        qWarning("QAudioInput: open error, invalid format.");
    } else if (settings.channelCount() <= 0) {
        qWarning("QAudioInput: open error, invalid number of channels (%d).",
                 settings.channelCount());
    } else if (settings.sampleSize() <= 0) {
        qWarning("QAudioInput: open error, invalid sample size (%d).",
                 settings.sampleSize());
    } else if (settings.frequency() < 8000 || settings.frequency() > 48000) {
        qWarning("QAudioInput: open error, frequency out of range (%d).", settings.frequency());
    } else if (buffer_size == 0) {

        buffer_size
                = (settings.frequency()
                * settings.channelCount()
                * settings.sampleSize()
                + 39) / 40;
        period_size = buffer_size / 5;
    } else {
        period_size = buffer_size / 5;
    }

    if (period_size == 0) {
        errorState = QAudio::OpenError;
        deviceState = QAudio::StoppedState;
        emit stateChanged(deviceState);
        return false;
    }

    timeStamp.restart();
    elapsedTimeOffset = 0;
    wfx.nSamplesPerSec = settings.frequency();
    wfx.wBitsPerSample = settings.sampleSize();
    wfx.nChannels = settings.channels();
    wfx.cbSize = 0;

    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

    QDataStream ds(&m_device, QIODevice::ReadOnly);
    quint32 deviceId;
    ds >> deviceId;

    if (waveInOpen(&hWaveIn, UINT_PTR(deviceId), &wfx,
                (DWORD_PTR)&waveInProc,
                (DWORD_PTR) this,
                CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
        errorState = QAudio::OpenError;
        deviceState = QAudio::StoppedState;
        emit stateChanged(deviceState);
        qWarning("QAudioInput: failed to open audio device");
        return false;
    }
    waveBlocks = allocateBlocks(period_size, buffer_size/period_size);
    waveBlockOffset = 0;

    if(waveBlocks == 0) {
        errorState = QAudio::OpenError;
        deviceState = QAudio::StoppedState;
        emit stateChanged(deviceState);
        qWarning("QAudioInput: failed to allocate blocks. open failed");
        return false;
    }

    mutex.lock();
    waveFreeBlockCount = buffer_size/period_size;
    mutex.unlock();

    for(int i=0; i<buffer_size/period_size; i++) {
        result = waveInAddBuffer(hWaveIn, &waveBlocks[i], sizeof(WAVEHDR));
        if(result != MMSYSERR_NOERROR) {
            qWarning("QAudioInput: failed to setup block %d,err=%d",i,result);
            errorState = QAudio::OpenError;
            deviceState = QAudio::StoppedState;
            emit stateChanged(deviceState);
            return false;
        }
    }
    result = waveInStart(hWaveIn);
    if(result) {
        qWarning("QAudioInput: failed to start audio input");
        errorState = QAudio::OpenError;
        deviceState = QAudio::StoppedState;
        emit stateChanged(deviceState);
        return false;
    }
    timeStampOpened.restart();
    elapsedTimeOffset = 0;
    totalTimeValue = 0;
    errorState  = QAudio::NoError;
    return true;
}

void QAudioInputPrivate::close()
{
    if(deviceState == QAudio::StoppedState)
        return;

    deviceState = QAudio::StoppedState;
    waveInReset(hWaveIn);
    waveInClose(hWaveIn);

    int count = 0;
    while(!finished && count < 500) {
        count++;
        Sleep(10);
    }

    mutex.lock();
    for(int i=0; i<waveFreeBlockCount; i++)
        waveInUnprepareHeader(hWaveIn,&waveBlocks[i],sizeof(WAVEHDR));
    freeBlocks(waveBlocks);
    mutex.unlock();
}

int QAudioInputPrivate::bytesReady() const
{
    if(period_size == 0 || buffer_size == 0)
        return 0;

    int buf = ((buffer_size/period_size)-waveFreeBlockCount)*period_size;
    if(buf < 0)
        buf = 0;
    return buf;
}

qint64 QAudioInputPrivate::read(char* data, qint64 len)
{
    bool done = false;

    char*  p = data;
    qint64 l = 0;
    qint64 written = 0;
    while(!done) {
        // Read in some audio data
        if(waveBlocks[header].dwBytesRecorded > 0 && waveBlocks[header].dwFlags & WHDR_DONE) {
            if(pullMode) {
                l = audioSource->write(waveBlocks[header].lpData + waveBlockOffset,
                        waveBlocks[header].dwBytesRecorded - waveBlockOffset);
#ifdef DEBUG_AUDIO
                qDebug()<<"IN: "<<waveBlocks[header].dwBytesRecorded<<", OUT: "<<l;
#endif
                if(l < 0) {
                    // error
                    qWarning("QAudioInput: IOError");
                    errorState = QAudio::IOError;

                } else if(l == 0) {
                    // cant write to IODevice
                    qWarning("QAudioInput: IOError, can't write to QIODevice");
                    errorState = QAudio::IOError;

                } else {
                    totalTimeValue += l;
                    errorState = QAudio::NoError;
                    if (deviceState != QAudio::ActiveState) {
                        deviceState = QAudio::ActiveState;
                        emit stateChanged(deviceState);
                    }
		    resuming = false;
                }
            } else {
                l = qMin<qint64>(len, waveBlocks[header].dwBytesRecorded - waveBlockOffset);
                // push mode
                memcpy(p, waveBlocks[header].lpData + waveBlockOffset, l);

                len -= l;

#ifdef DEBUG_AUDIO
                qDebug()<<"IN: "<<waveBlocks[header].dwBytesRecorded<<", OUT: "<<l;
#endif
                totalTimeValue += l;
                errorState = QAudio::NoError;
                if (deviceState != QAudio::ActiveState) {
                    deviceState = QAudio::ActiveState;
                    emit stateChanged(deviceState);
                }
		resuming = false;
            }
        } else {
            //no data, not ready yet, next time
            break;
        }

        if (l < waveBlocks[header].dwBytesRecorded - waveBlockOffset) {
            waveBlockOffset += l;
            done = true;
        } else {
            waveBlockOffset = 0;

            waveInUnprepareHeader(hWaveIn,&waveBlocks[header], sizeof(WAVEHDR));

            mutex.lock();
            waveFreeBlockCount++;
            mutex.unlock();

            waveBlocks[header].dwBytesRecorded=0;
            waveBlocks[header].dwFlags = 0L;
            result = waveInPrepareHeader(hWaveIn,&waveBlocks[header], sizeof(WAVEHDR));
            if(result != MMSYSERR_NOERROR) {
                result = waveInPrepareHeader(hWaveIn,&waveBlocks[header], sizeof(WAVEHDR));
                qWarning("QAudioInput: failed to prepare block %d,err=%d",header,result);
                errorState = QAudio::IOError;

                mutex.lock();
                waveFreeBlockCount--;
                mutex.unlock();

                return 0;
            }
            result = waveInAddBuffer(hWaveIn, &waveBlocks[header], sizeof(WAVEHDR));
            if(result != MMSYSERR_NOERROR) {
                qWarning("QAudioInput: failed to setup block %d,err=%d",header,result);
                errorState = QAudio::IOError;

                mutex.lock();
                waveFreeBlockCount--;
                mutex.unlock();

                return 0;
            }
            header++;
            if(header >= buffer_size/period_size)
                header = 0;
            p+=l;

            mutex.lock();
            if(!pullMode) {
                if(len < period_size || waveFreeBlockCount == buffer_size/period_size)
                    done = true;
            } else {
                if(waveFreeBlockCount == buffer_size/period_size)
                    done = true;
            }
            mutex.unlock();
        }

	written+=l;
    }
#ifdef DEBUG_AUDIO
    qDebug()<<"read in len="<<written;
#endif
    return written;
}

void QAudioInputPrivate::resume()
{
    if(deviceState == QAudio::SuspendedState) {
        deviceState = QAudio::ActiveState;
        for(int i=0; i<buffer_size/period_size; i++) {
            result = waveInAddBuffer(hWaveIn, &waveBlocks[i], sizeof(WAVEHDR));
            if(result != MMSYSERR_NOERROR) {
                qWarning("QAudioInput: failed to setup block %d,err=%d",i,result);
                errorState = QAudio::OpenError;
                deviceState = QAudio::StoppedState;
                emit stateChanged(deviceState);
                return;
            }
        }

        mutex.lock();
        waveFreeBlockCount = buffer_size/period_size;
        mutex.unlock();

        header = 0;
	resuming = true;
        waveBlockOffset = 0;
        waveInStart(hWaveIn);
        QTimer::singleShot(20,this,SLOT(feedback()));
        emit stateChanged(deviceState);
    }
}

void QAudioInputPrivate::setBufferSize(int value)
{
    buffer_size = value;
}

int QAudioInputPrivate::bufferSize() const
{
    return buffer_size;
}

int QAudioInputPrivate::periodSize() const
{
    return period_size;
}

void QAudioInputPrivate::setNotifyInterval(int ms)
{
    intervalTime = qMax(0, ms);
}

int QAudioInputPrivate::notifyInterval() const
{
    return intervalTime;
}

qint64 QAudioInputPrivate::processedUSecs() const
{
    if (deviceState == QAudio::StoppedState)
        return 0;
    qint64 result = qint64(1000000) * totalTimeValue /
        (settings.channels()*(settings.sampleSize()/8)) /
        settings.frequency();

    return result;
}

void QAudioInputPrivate::suspend()
{
    if(deviceState == QAudio::ActiveState) {
        waveInReset(hWaveIn);
        deviceState = QAudio::SuspendedState;
        emit stateChanged(deviceState);
    }
}

void QAudioInputPrivate::feedback()
{
#ifdef DEBUG_AUDIO
    QTime now(QTime::currentTime());
    qDebug()<<now.second()<<"s "<<now.msec()<<"ms :feedback() INPUT "<<this;
#endif
    if(!(deviceState==QAudio::StoppedState||deviceState==QAudio::SuspendedState))
        QMetaObject::invokeMethod(this, "deviceReady", Qt::QueuedConnection);
}

bool QAudioInputPrivate::deviceReady()
{
    bytesAvailable = bytesReady();
#ifdef DEBUG_AUDIO
    QTime now(QTime::currentTime());
    qDebug()<<now.second()<<"s "<<now.msec()<<"ms :deviceReady() INPUT";
#endif
    if(deviceState != QAudio::ActiveState && deviceState != QAudio::IdleState)
        return true;

    if(pullMode) {
        // reads some audio data and writes it to QIODevice
        read(0, buffer_size);
    } else {
        // emits readyRead() so user will call read() on QIODevice to get some audio data
	InputPrivate* a = qobject_cast<InputPrivate*>(audioSource);
	a->trigger();
    }

    if(intervalTime && (timeStamp.elapsed() + elapsedTimeOffset) > intervalTime) {
        emit notify();
        elapsedTimeOffset = timeStamp.elapsed() + elapsedTimeOffset - intervalTime;
        timeStamp.restart();
    }
    return true;
}

qint64 QAudioInputPrivate::elapsedUSecs() const
{
    if (deviceState == QAudio::StoppedState)
        return 0;

    return timeStampOpened.elapsed()*1000;
}

void QAudioInputPrivate::reset()
{
    stop();
    if (period_size > 0)
        waveFreeBlockCount = buffer_size / period_size;
}

InputPrivate::InputPrivate(QAudioInputPrivate* audio)
{
    audioDevice = qobject_cast<QAudioInputPrivate*>(audio);
}

InputPrivate::~InputPrivate() {}

qint64 InputPrivate::readData( char* data, qint64 len)
{
    // push mode, user read() called
    if(audioDevice->deviceState != QAudio::ActiveState &&
            audioDevice->deviceState != QAudio::IdleState)
        return 0;
    // Read in some audio data
    return audioDevice->read(data,len);
}

qint64 InputPrivate::writeData(const char* data, qint64 len)
{
    Q_UNUSED(data)
    Q_UNUSED(len)

    emit readyRead();
    return 0;
}

void InputPrivate::trigger()
{
    emit readyRead();
}

QT_END_NAMESPACE

#include "moc_qaudioinput_win32_p.cpp"
