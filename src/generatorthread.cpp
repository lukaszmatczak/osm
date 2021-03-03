/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "generatorthread.h"
#include "audio/client.h"

#include "whitenoise.h"
#include "pinknoise.h"
#include "sinnoise.h"
#include "sinsweep.h"

GeneratorThread::GeneratorThread(QObject *parent) :
    QThread(parent),
    m_deviceId(audio::Client::getInstance()->defaultDeviceId(audio::Plugin::Direction::Output)),
    m_audioStream(nullptr),
    m_sources(),
    m_gain(-6.f), m_duration(1.f),
    m_type(0),
    m_frequency(1000),
    m_startFrequency(20),
    m_endFrequency(20000),
    m_channelCount(1),
    m_channel(0),
    m_aux(1),
    m_enabled(false)
{
    start();
    QObject::moveToThread(this);
}

GeneratorThread::~GeneratorThread()
{
    for (auto &m_source : m_sources) {
        m_source->close();
    }

    if (m_audioStream) {
        m_audioStream->close();
    }
}

audio::DeviceInfo::Id GeneratorThread::deviceId() const
{
    return m_deviceId;
}

void GeneratorThread::setDeviceId(audio::DeviceInfo::Id deviceId)
{
    if (m_deviceId != deviceId) {
        m_deviceId = deviceId;
        updateAudio();
        emit deviceIdChanged(m_deviceId);
    }
}

float GeneratorThread::duration() const
{
    return m_duration;
}

void GeneratorThread::setDuration(float duration)
{
    if (!qFuzzyCompare(duration, m_duration)) {
        m_duration = duration;
        emit durationChanged(m_duration);
    }
}

void GeneratorThread::init()
{
    m_sources << new PinkNoise(this);
    m_sources << new WhiteNoise(this);
    m_sources << new SinNoise(this);
    m_sources << new SinSweep(this);
    connect(this, SIGNAL(finished()), this, SLOT(finish()));
}
/*
 * finishes audio in current thread
 */
void GeneratorThread::finish()
{
    setEnabled(false);
}
void GeneratorThread::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        updateAudio();
        if (m_type == 3 && m_enabled == false)
            qobject_cast<SinSweep *>(m_sources[3])->setFrequency(m_startFrequency);
        emit enabledChanged(m_enabled);
    }
}
void GeneratorThread::setType(int type)
{
    if (m_type != type) {
        m_type = type;
        updateAudio();
        emit typeChanged(m_type);
    }
}
void GeneratorThread::updateAudio()
{
    if (m_audioStream) {
        m_audioStream->close();
    }
    m_audioStream = nullptr;

    if (m_enabled) {
        m_sources[m_type]->setGain(m_gain);
        m_sources[m_type]->setChanel(m_channel);
        m_sources[m_type]->setAux(m_aux);

        audio::Format format = audio::Client::getInstance()->deviceOutputFormat(m_deviceId);
        m_sources[m_type]->setChanelCount(format.channelCount);
        m_sources[m_type]->setSamplerate(format.sampleRate);

        m_audioStream = audio::Client::getInstance()->openOutput(m_deviceId, m_sources[m_type], format);
    }
}
QVariant GeneratorThread::getAvailableTypes() const
{
    QStringList nameList;
    foreach (OutputDevice *o, m_sources) {
        nameList << o->name();
    }
    return QVariant::fromValue(nameList);
}
void GeneratorThread::setFrequency(int frequency)
{
    if (m_frequency != frequency) {
        m_frequency = frequency;
        emit frequencyChanged(m_frequency);
    }
}
void GeneratorThread::setStartFrequency(int startFrequency)
{
    if (m_startFrequency != startFrequency) {
        m_startFrequency = startFrequency;
        emit startFrequencyChanged(m_startFrequency);
    }
}
void GeneratorThread::setEndFrequency(int endFrequency)
{
    if (m_endFrequency != endFrequency) {
        m_endFrequency = endFrequency;
        emit endFrequencyChanged(m_endFrequency);
    }
}
void GeneratorThread::setGain(float gain)
{
    if (!qFuzzyCompare(gain, m_gain)) {
        m_gain = gain;
        emit gainChanged(gain);
    }
}
void GeneratorThread::setChannel(int chanel)
{
    if (m_channel != chanel) {
        m_channel = chanel;
        m_sources[m_type]->setChanel(m_channel);
        emit channelChanged(m_channel);
    }
}
void GeneratorThread::setAux(int chanel)
{
    if (m_aux != chanel) {
        m_aux = chanel;
        m_sources[m_type]->setAux(m_aux);
        emit auxChanged(m_aux);
    }
}
