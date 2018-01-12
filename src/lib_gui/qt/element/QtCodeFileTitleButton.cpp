#include "qt/element/QtCodeFileTitleButton.h"

#include "utility/file/FileSystem.h"
#include "utility/messaging/type/MessageActivateFile.h"
#include "utility/messaging/type/MessageProjectEdit.h"

#include "Application.h"
#include "project/Project.h"
#include "qt/utility/QtContextMenu.h"
#include "qt/utility/utilityQt.h"
#include "settings/ColorScheme.h"
#include "utility/ResourcePaths.h"

QtCodeFileTitleButton::QtCodeFileTitleButton(QWidget* parent)
	: QPushButton(parent)
	, m_isComplete(true)
{
	setObjectName("title_button");
	minimumSizeHint(); // force font loading
	setAttribute(Qt::WA_LayoutUsesWidgetRect); // fixes layouting on Mac

	setFixedHeight(std::max(fontMetrics().height() * 1.2, 28.0));
	setSizePolicy(sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);

	setIconSize(QSize(16, 16));

	connect(this, &QtCodeFileTitleButton::clicked, this, &QtCodeFileTitleButton::clickedTitle);
}

QtCodeFileTitleButton::~QtCodeFileTitleButton()
{
}

void QtCodeFileTitleButton::setFilePath(const FilePath& filePath)
{
	setEnabled(true);

	if (m_filePath.empty())
	{
		std::string text = ResourcePaths::getGuiPath().str() + "code_view/images/file.png";
		setIcon(utility::colorizePixmap(
			QPixmap(text.c_str()),
			ColorScheme::getInstance()->getColor("code/file/title/icon").c_str()
		));
	}

	m_filePath = filePath;
}

void QtCodeFileTitleButton::setModificationTime(const TimeStamp modificationTime)
{
	if (modificationTime.isValid())
	{
		m_modificationTime = modificationTime;
	}
}

void QtCodeFileTitleButton::setIsComplete(bool isComplete)
{
	if (m_isComplete == isComplete)
	{
		return;
	}

	m_isComplete = isComplete;
	setProperty("complete", isComplete);

	if (!isComplete)
	{
		FilePath hatchingFilePath(ResourcePaths::getGuiPath().str() + "code_view/images/pattern_" +
			ColorScheme::getInstance()->getColor("code/file/title/hatching") + ".png"
		);

		setStyleSheet((
			"#title_button { background-image: url(" + hatchingFilePath.str() + "); }"
		).c_str());
	}
	else
	{
		setStyleSheet("");
	}
}

void QtCodeFileTitleButton::setProject(const std::string& name)
{
	m_filePath = FilePath();

	setText(name.c_str());
	setToolTip("edit project");

	std::string text = ResourcePaths::getGuiPath().str() + "code_view/images/edit.png";

	setIcon(utility::colorizePixmap(
		QPixmap(text.c_str()),
		ColorScheme::getInstance()->getColor("code/file/title/icon").c_str()
	));
}

void QtCodeFileTitleButton::updateTexts()
{
	if (m_filePath.empty())
	{
		return;
	}

	std::string title = m_filePath.fileName();
	std::string toolTip = "file: " + m_filePath.str();

	if ((!m_filePath.recheckExists()) ||
		(FileSystem::getLastWriteTime(m_filePath) > m_modificationTime))
	{
		title += "*";
		toolTip = "out of date " + toolTip;
	}

	if (!m_isComplete)
	{
		toolTip = "incomplete " + toolTip;
	}

	setText(title.c_str());
	setToolTip(toolTip.c_str());
}

void QtCodeFileTitleButton::updateFromOther(const QtCodeFileTitleButton* other)
{
	if (!other->m_filePath.empty())
	{
		setFilePath(other->m_filePath);
	}
	else
	{
		setProject(other->text().toStdString());
	}

	setModificationTime(other->m_modificationTime);
	setIsComplete(other->m_isComplete);
	updateTexts();
}

void QtCodeFileTitleButton::contextMenuEvent(QContextMenuEvent* event)
{
	FilePath path = m_filePath;
	if (path.empty())
	{
		Project* currentProject = Application::getInstance()->getCurrentProject().get();
		if (!currentProject)
		{
			return;
		}

		path = currentProject->getProjectSettingsFilePath();
	}

	QtContextMenu menu(event, this);
	menu.addSeparator();
	menu.addFileActions(path);
	menu.show();
}

void QtCodeFileTitleButton::clickedTitle()
{
	if (!m_filePath.empty())
	{
		MessageActivateFile(m_filePath).dispatch();
	}
	else if (text().size())
	{
		MessageProjectEdit().dispatch();
	}
}
