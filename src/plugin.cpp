// SPDX-License-Identifier: GPL-3.0-or-later
#include "Effect.h"

namespace KWin
{
Q_NAMESPACE

KWIN_EFFECT_FACTORY_SUPPORTED(CrtEffect, "metadata.json", return CrtEffect::supported();)
}

#include "plugin.moc"
