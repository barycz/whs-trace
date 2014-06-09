#pragma once
#include "history.h"
#include <QComboBox>

inline void syncHistoryToWidget (QComboBox * c, History<QString> const & h)
{
	if (c)
	{
		c->clear();
		for (size_t i = 0, ie = h.size(); i < ie; ++i)
			c->addItem(h[i]);
		if (h.m_current_item < h.size())
			c->setCurrentIndex(h.m_current_item);
	}
}

inline void mentionStringInHistory_NoRef (QString const & str, QComboBox * c, History<QString> & h)
{
	if (str.isEmpty())
		return;

	h.insert_no_refcount(str);
	syncHistoryToWidget(c, h);
	//ui->presetComboBox->setCurrentIndex(ui->presetComboBox->findText(str));
}

inline void mentionStringInHistory_Ref (QString const & str, QComboBox * c, History<QString> & h)
{
	if (str.isEmpty())
		return;

	h.insert(str);
	syncHistoryToWidget(c, h);
	//ui->presetComboBox->setCurrentIndex(ui->presetComboBox->findText(str));
}

inline void removeStringFromHistory (QString const & str, QComboBox * c, History<QString> & h)
{
	if (str.isEmpty())
		return;

	h.remove(str);
	syncHistoryToWidget(c, h);
}

