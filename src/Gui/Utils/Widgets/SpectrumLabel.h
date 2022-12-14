#ifndef SPECTRUMLABEL_H
#define SPECTRUMLABEL_H

#include "Interfaces/Engine/AudioDataReceiver.h"

#include "Utils/Pimpl.h"

#include <QLabel>
#include <vector>

class SpectrumDataProvider;
class SpectrumLabel :
	public QLabel,
	public Engine::SpectrumDataReceiver
{
	Q_OBJECT
	PIMPL(SpectrumLabel)

	signals:
		void sigPixmapChanged();

	public:
		SpectrumLabel(SpectrumDataProvider* dataProvider, QWidget* parent);
		~SpectrumLabel() override;

		void setSpectrum(const std::vector<float>& spectrum) override;
		bool isActive() const override;
};

#endif // SPECTRUMLABEL_H
