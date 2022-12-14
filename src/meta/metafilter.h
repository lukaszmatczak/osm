/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
#ifndef META_FILTER_H
#define META_FILTER_H

#include <QObject>
#include "metabase.h"
#include "metameasurement.h"

namespace meta {

class Filter : public Base
{
    Q_GADGET
    QML_ELEMENT

public:
    enum Type {
        ButterworthLPF,
        ButterworthHPF,
        LinkwitzRileyLPF,
        LinkwitzRileyHPF,
        BesselLPF,
        BesselHPF
    };
    Q_ENUM(Type)

    Filter();


    Type type() const;
    void setType(Type newType);
    QVariant getAvailableTypes() const;

    unsigned int sampleRate() const;
    void setSampleRate(unsigned int newSampleRate);

    const Measurement::Mode &mode() const;
    void setMode(const Measurement::Mode &newMode);
    static QVariant getAvailableModes();

    float cornerFrequency() const;
    void setCornerFrequency(float newFrequency);

    unsigned int order() const;
    void setOrder(unsigned int newOrder);
    QVariant getAvailableOrders();

    Q_INVOKABLE virtual chart::Source *store() noexcept = 0;

//virtual signals:
    virtual void sampleRateChanged(unsigned int) = 0;
    virtual void typeChanged(meta::Filter::Type) = 0;
    virtual void modeChanged(meta::Measurement::Mode) = 0;
    virtual void cornerFrequencyChanged(float) = 0;
    virtual void orderChanged(unsigned int) = 0;

    static const std::map<Type, QString> m_typeMap;
    static const std::map<Type, QString> m_typeShortMap;

private:
    Type m_type;
    Measurement::Mode m_mode;
    unsigned int m_sampleRate, m_order;
    float m_cornerFrequency;
};

} // namespace meta
Q_DECLARE_METATYPE(meta::Filter::Type)

#endif // META_FILTER_H