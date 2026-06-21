// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <KCModule>

namespace CrtTrinitron
{

class KCM final : public KCModule
{
    Q_OBJECT

public:
    explicit KCM(QObject *parent, const KPluginMetaData &args);

public Q_SLOTS:
    void save() override;
};

}
