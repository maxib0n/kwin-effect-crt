// SPDX-License-Identifier: GPL-3.0-or-later
#include "KCM.h"
#include "Config.h"

#include <KLocalizedString>
#include <QComboBox>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QSpinBox>

namespace CrtTrinitron
{

namespace
{
QDoubleSpinBox *dbl(const QString &name, double mn, double mx, double step)
{
    auto *s = new QDoubleSpinBox();
    s->setObjectName(name);
    s->setRange(mn, mx);
    s->setSingleStep(step);
    s->setDecimals(2);
    return s;
}
QSpinBox *intpx(const QString &name, int mn, int mx)
{
    auto *s = new QSpinBox();
    s->setObjectName(name);
    s->setRange(mn, mx);
    s->setSuffix(i18n(" px"));
    return s;
}
QComboBox *combo(const QString &name, const QStringList &items)
{
    auto *c = new QComboBox();
    c->setObjectName(name);
    c->addItems(items);
    return c;
}
QFrame *hsep()
{
    auto *f = new QFrame();
    f->setFrameShape(QFrame::HLine);
    f->setFrameShadow(QFrame::Sunken);
    return f;
}
}

KCM::KCM(QObject *parent, const KPluginMetaData &args)
    : KCModule(parent, args)
{
    auto *form = new QFormLayout(widget());

    form->addRow(i18n("CRT type:"),
                 combo(QStringLiteral("kcfg_MaskType"),
                       {i18n("Aperture grille (Trinitron)"), i18n("Shadow mask"), i18n("Slot mask")}));
    form->addRow(i18n("Mask strength:"), dbl(QStringLiteral("kcfg_MaskStrength"), 0.0, 1.0, 0.01));
    form->addRow(i18n("Mask size:"), intpx(QStringLiteral("kcfg_MaskSize"), 2, 12));

    form->addRow(hsep());
    form->addRow(i18n("Phosphor:"),
                 combo(QStringLiteral("kcfg_ColorScheme"),
                       {i18n("RGB (color)"), i18n("Green (P1)"), i18n("Amber (P3)"), i18n("White (B/W)")}));
    form->addRow(i18n("Saturation:"), dbl(QStringLiteral("kcfg_Saturation"), 0.0, 2.0, 0.01));

    form->addRow(hsep());
    form->addRow(i18n("Scanline strength:"), dbl(QStringLiteral("kcfg_ScanlineStrength"), 0.0, 1.0, 0.01));
    form->addRow(i18n("Scanline size:"), intpx(QStringLiteral("kcfg_ScanlineSize"), 2, 16));
    form->addRow(i18n("Scanline beam:"), dbl(QStringLiteral("kcfg_ScanlineBeam"), 0.3, 3.0, 0.05));

    form->addRow(hsep());
    form->addRow(i18n("Glow:"), dbl(QStringLiteral("kcfg_Glow"), 0.0, 0.6, 0.01));
    form->addRow(i18n("Glow radius:"), dbl(QStringLiteral("kcfg_GlowRadius"), 0.5, 3.0, 0.1));
    form->addRow(i18n("Curvature:"), dbl(QStringLiteral("kcfg_Curvature"), 0.0, 0.4, 0.01));
    form->addRow(i18n("Vignette:"), dbl(QStringLiteral("kcfg_Vignette"), 0.0, 1.0, 0.01));

    form->addRow(hsep());
    form->addRow(i18n("Brightness:"), dbl(QStringLiteral("kcfg_Brightness"), 0.5, 2.0, 0.01));
    form->addRow(i18n("Contrast:"), dbl(QStringLiteral("kcfg_Contrast"), 0.5, 1.8, 0.01));
    form->addRow(i18n("Gamma:"), dbl(QStringLiteral("kcfg_Gamma"), 0.6, 1.6, 0.01));

    auto *hint = new QLabel(i18n("Brightness 1.0 = system. Keep mask strength low (0.10–0.20) so thin text stays legible."));
    hint->setWordWrap(true);
    hint->setEnabled(false);
    form->addRow(hint);

    addConfig(Config::self(), widget());
}

void KCM::save()
{
    KCModule::save();
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.kde.KWin"), QStringLiteral("/Effects"),
        QStringLiteral("org.kde.kwin.Effects"), QStringLiteral("reconfigureEffect"));
    msg << QStringLiteral("kwin_effect_crt");
    QDBusConnection::sessionBus().send(msg);
}

}
