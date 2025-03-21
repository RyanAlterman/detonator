// Copyright (C) 2020-2021 Sami Väisänen
// Copyright (C) 2020-2021 Ensisoft http://www.ensisoft.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#define LOGTAG "gui"

#include "config.h"

#include "warnpush.h"
#  include <QApplication>
#  include <QStringList>
#  include <QFileInfo>
#  include <QFileDialog>
#  include <QDir>
#  include <QStyle>
#  include <QStyleFactory>
#include "warnpop.h"

#include "editor/app/eventlog.h"
#include "editor/gui/dlgsettings.h"
#include "editor/gui/utility.h"

namespace gui
{

DlgSettings::DlgSettings(QWidget* parent, AppSettings& settings,
    ScriptWidget::Settings& script,
    MainWidget::UISettings& widget)
    : QDialog(parent)
    , mSettings(settings)
    , mScriptSettings(script)
    , mWidgetSettings(widget)
    , mAssistant(nullptr)
{

    auto* layout = new QPlainTextDocumentLayout(&mSampleCode);
    layout->setParent(this);
    mSampleCode.setDocumentLayout(layout);
    mSampleCode.setPlainText(R"(-- this is a comment

local my_integer = 123
local my_string  = 'hello'
local my_float   = 123.0

function MyFunction()
  if my_integer == 42 then
     print('hello')
  end
end

)");

    mAssistant.SetTheme(mScriptSettings.theme);
    mAssistant.ParseSource(mSampleCode);
    mAssistant.ApplyHighlight(mSampleCode);

    mUI.setupUi(this);
    PopulateFromEnum<MainWidget::GridDensity>(mUI.cmbGrid);
    PopulateFromEnum<GfxWindow::MouseCursor>(mUI.cmbMouseCursor);
    PopulateFromEnum<QTabWidget::TabPosition>(mUI.cmbTabPosition);
    PopulateFromEnum<TextEditor::Keymap>(mUI.cmbKeymap);
    PopulateFontSizes(mUI.editorFontSize);
    PopulateQtStyles(mUI.cmbStyle);

    mUI.editorTheme->addItem("Monokai");
    mUI.editorTheme->addItem("Solar Flare");
    mUI.editorTheme->addItem("Pastel Dream");
    mUI.editorTheme->addItem("Dark Mirage");
    mUI.editorTheme->addItem("Cyber Flux");
    mUI.editorTheme->addItem("Orange Crush");
    mUI.code->SetDocument(&mSampleCode);
    mUI.code->SetSettings(script.editor_settings);
    mUI.code->ApplySettings();

    connect(mUI.editorTheme, &QComboBox::currentTextChanged, this, [this](QString theme) {
        mAssistant.SetTheme(theme);
        mAssistant.RemoveHighlight(mSampleCode);
        mAssistant.ApplyHighlight(mSampleCode);
    });

    // general application settings.
    SetUIValue(mUI.edtImageEditorExecutable,    settings.image_editor_executable);
    SetUIValue(mUI.edtImageEditorArguments,     settings.image_editor_arguments);
    SetUIValue(mUI.edtShaderEditorExecutable,   settings.shader_editor_executable);
    SetUIValue(mUI.edtShaderEditorArguments,    settings.shader_editor_arguments);
    SetUIValue(mUI.edtScriptEditorExecutable,   settings.script_editor_executable);
    SetUIValue(mUI.edtScriptEditorArguments,    settings.script_editor_arguments);
    SetUIValue(mUI.edtAudioEditorExecutable,    settings.audio_editor_executable);
    SetUIValue(mUI.edtAudioEditorArguments,     settings.audio_editor_arguments);
    SetUIValue(mUI.cmbWinOrTab,                 settings.default_open_win_or_tab);
    SetUIValue(mUI.cmbStyle,                    settings.style_name);
    SetUIValue(mUI.spinFrameDelay,              settings.frame_delay);
    SetUIValue(mUI.cmbMouseCursor,              settings.mouse_cursor);
    SetUIValue(mUI.cmbTabPosition,              settings.main_tab_position);
    SetUIValue(mUI.chkSaveAutomatically,        settings.save_automatically_on_play);
    SetUIValue(mUI.chkVSYNC,                    settings.vsync);
    SetUIValue(mUI.edtPythonExecutable,         settings.python_executable);
    SetUIValue(mUI.edtEmscriptenPath,           settings.emsdk);
    SetUIValue(mUI.clearColor,                  settings.clear_color);
    SetUIValue(mUI.gridColor,                   settings.grid_color);
    SetUIValue(mUI.vcsExecutable,               settings.vcs_executable);
    SetUIValue(mUI.vcsAddFile,                  settings.vcs_cmd_add_file);
    SetUIValue(mUI.vcsDelFile,                  settings.vcs_cmd_del_file);
    SetUIValue(mUI.vcsCommitFile,               settings.vcs_cmd_commit_file);
    SetUIValue(mUI.vcsListFiles,                settings.vcs_cmd_list_files);
    SetUIValue(mUI.vcsIgnoreList,               settings.vcs_ignore_list);
    // main widget settings, grid,color etc.
    SetUIValue(mUI.cmbGrid,                     widget.grid);
    SetUIValue(mUI.zoom,                        widget.zoom);
    SetUIValue(mUI.chkShowGrid,                 widget.show_grid);
    SetUIValue(mUI.chkShowOrigin,               widget.show_origin);
    SetUIValue(mUI.chkShowViewport,             widget.show_viewport);
    SetUIValue(mUI.chkSnapToGrid,               widget.snap_to_grid);

    // Lua script editor settings
    SetUIValue(mUI.editorTheme,                 script.theme);
    SetUIValue(mUI.edtLuaFormatterExec,         script.lua_formatter_exec);
    SetUIValue(mUI.edtLuaFormatterArgs,         script.lua_formatter_args);
    SetUIValue(mUI.editorFormatOnSave,          script.lua_format_on_save);
    SetUIValue(mUI.chkUseCodeHeuristics,        script.use_code_heuristics);
    SetUIValue(mUI.cmbKeymap,                   script.editor_settings.keymap);
    SetUIValue(mUI.chkUseCodeCompletion,        script.editor_settings.use_code_completer);
    SetUIValue(mUI.editorShowLineNumbers,       script.editor_settings.show_line_numbers);
    SetUIValue(mUI.editorHightlightCurrentLine, script.editor_settings.highlight_current_line);
    SetUIValue(mUI.editorHightlightSyntax,      script.editor_settings.highlight_syntax);
    SetUIValue(mUI.editorInsertSpaces,          script.editor_settings.replace_tabs_with_spaces);
    SetUIValue(mUI.editorFontSize,              script.editor_settings.font_size);
    SetUIValue(mUI.editorFontName,              script.editor_settings.font_description);
}

void DlgSettings::UpdateSampleCode()
{
    TextEditor::Settings editor_settings;
    GetUIValue(mUI.chkUseCodeCompletion,        &editor_settings.use_code_completer);
    GetUIValue(mUI.editorShowLineNumbers,       &editor_settings.show_line_numbers);
    GetUIValue(mUI.editorHightlightCurrentLine, &editor_settings.highlight_current_line);
    GetUIValue(mUI.editorHightlightSyntax,      &editor_settings.highlight_syntax);
    GetUIValue(mUI.editorInsertSpaces,          &editor_settings.replace_tabs_with_spaces);
    GetUIValue(mUI.editorFontSize,              &editor_settings.font_size);
    GetUIValue(mUI.editorFontName,              &editor_settings.font_description);
    mUI.code->SetSettings(editor_settings);
    mUI.code->ApplySettings();
    if (editor_settings.highlight_syntax)
        mAssistant.ApplyHighlight(mSampleCode);
    else mAssistant.RemoveHighlight(mSampleCode);

    mUI.code->update();
}

void DlgSettings::on_btnAccept_clicked()
{
    // general settings
    GetUIValue(mUI.edtImageEditorExecutable,    &mSettings.image_editor_executable);
    GetUIValue(mUI.edtImageEditorArguments,     &mSettings.image_editor_arguments);
    GetUIValue(mUI.edtShaderEditorExecutable,   &mSettings.shader_editor_executable);
    GetUIValue(mUI.edtShaderEditorArguments,    &mSettings.shader_editor_arguments);
    GetUIValue(mUI.edtScriptEditorExecutable,   &mSettings.script_editor_executable);
    GetUIValue(mUI.edtScriptEditorArguments,    &mSettings.script_editor_arguments);
    GetUIValue(mUI.edtAudioEditorExecutable,    &mSettings.audio_editor_executable);
    GetUIValue(mUI.edtAudioEditorArguments,     &mSettings.audio_editor_arguments);
    GetUIValue(mUI.cmbWinOrTab,                 &mSettings.default_open_win_or_tab);
    GetUIValue(mUI.cmbStyle,                    &mSettings.style_name);
    GetUIValue(mUI.chkSaveAutomatically,        &mSettings.save_automatically_on_play);
    GetUIValue(mUI.spinFrameDelay,              &mSettings.frame_delay);
    GetUIValue(mUI.cmbMouseCursor,              &mSettings.mouse_cursor);
    GetUIValue(mUI.cmbTabPosition,              &mSettings.main_tab_position);
    GetUIValue(mUI.chkVSYNC,                    &mSettings.vsync);
    GetUIValue(mUI.edtPythonExecutable,         &mSettings.python_executable);
    GetUIValue(mUI.edtEmscriptenPath,           &mSettings.emsdk);
    GetUIValue(mUI.clearColor,                  &mSettings.clear_color);
    GetUIValue(mUI.gridColor,                   &mSettings.grid_color);
    GetUIValue(mUI.vcsExecutable,               &mSettings.vcs_executable);
    GetUIValue(mUI.vcsAddFile,                  &mSettings.vcs_cmd_add_file);
    GetUIValue(mUI.vcsDelFile,                  &mSettings.vcs_cmd_del_file);
    GetUIValue(mUI.vcsCommitFile,               &mSettings.vcs_cmd_commit_file);
    GetUIValue(mUI.vcsListFiles,                &mSettings.vcs_cmd_list_files);
    GetUIValue(mUI.vcsIgnoreList,               &mSettings.vcs_ignore_list);
    // main widget settings, grid, color etc.
    GetUIValue(mUI.cmbGrid,                     &mWidgetSettings.grid);
    GetUIValue(mUI.zoom,                        &mWidgetSettings.zoom);
    GetUIValue(mUI.chkShowGrid,                 &mWidgetSettings.show_grid);
    GetUIValue(mUI.chkShowOrigin,               &mWidgetSettings.show_origin);
    GetUIValue(mUI.chkShowViewport,             &mWidgetSettings.show_viewport);
    GetUIValue(mUI.chkSnapToGrid,               &mWidgetSettings.snap_to_grid);
    // lua script editor settings
    GetUIValue(mUI.editorTheme,                 &mScriptSettings.theme);
    GetUIValue(mUI.edtLuaFormatterExec,         &mScriptSettings.lua_formatter_exec);
    GetUIValue(mUI.edtLuaFormatterArgs,         &mScriptSettings.lua_formatter_args);
    GetUIValue(mUI.editorFormatOnSave,          &mScriptSettings.lua_format_on_save);
    GetUIValue(mUI.chkUseCodeHeuristics,        &mScriptSettings.use_code_heuristics);
    GetUIValue(mUI.cmbKeymap,                   &mScriptSettings.editor_settings.keymap);
    GetUIValue(mUI.chkUseCodeCompletion,        &mScriptSettings.editor_settings.use_code_completer);
    GetUIValue(mUI.editorShowLineNumbers,       &mScriptSettings.editor_settings.show_line_numbers);
    GetUIValue(mUI.editorHightlightCurrentLine, &mScriptSettings.editor_settings.highlight_current_line);
    GetUIValue(mUI.editorHightlightSyntax,      &mScriptSettings.editor_settings.highlight_syntax);
    GetUIValue(mUI.editorInsertSpaces,          &mScriptSettings.editor_settings.replace_tabs_with_spaces);
    GetUIValue(mUI.editorFontSize,              &mScriptSettings.editor_settings.font_size);
    GetUIValue(mUI.editorFontName,              &mScriptSettings.editor_settings.font_description);

    accept();
}
void DlgSettings::on_btnCancel_clicked()
{
    reject();
}

void DlgSettings::on_btnSelectImageEditor_clicked()
{
    QString filter;

    #if defined(WINDOWS_OS)
        filter = "Executables (*.exe)";
    #endif

    const QString& executable = QFileDialog::getOpenFileName(this,
        tr("Select Application"), QString(), filter);
    if (executable.isEmpty())
        return;

    const QFileInfo info(executable);
    mUI.edtImageEditorExecutable->setText(QDir::toNativeSeparators(executable));
    mUI.edtImageEditorExecutable->setCursorPosition(0);
}

void DlgSettings::on_btnSelectShaderEditor_clicked()
{
    QString filter;

    #if defined(WINDOWS_OS)
        filter = "Executables (*.exe)";
    #endif

    const QString& executable = QFileDialog::getOpenFileName(this,
        tr("Select Application"), QString(), filter);
    if (executable.isEmpty())
        return;

    const QFileInfo info(executable);
    mUI.edtShaderEditorExecutable->setText(QDir::toNativeSeparators(executable));
    mUI.edtShaderEditorExecutable->setCursorPosition(0);
}

void DlgSettings::on_btnSelectScriptEditor_clicked()
{
    QString filter;

#if defined(WINDOWS_OS)
    filter = "Executables (*.exe)";
#endif

    const QString& executable = QFileDialog::getOpenFileName(this,
      tr("Select Application"), QString(), filter);
    if (executable.isEmpty())
        return;

    const QFileInfo info(executable);
    mUI.edtScriptEditorExecutable->setText(QDir::toNativeSeparators(executable));
    mUI.edtScriptEditorExecutable->setCursorPosition(0);
}

void DlgSettings::on_btnSelectAudioEditor_clicked()
{
    QString filter;

#if defined(WINDOWS_OS)
    filter = "Executables (*.exe)";
#endif

    const QString& executable = QFileDialog::getOpenFileName(this,
      tr("Select Application"), QString(), filter);
    if (executable.isEmpty())
        return;

    const QFileInfo info(executable);
    mUI.edtAudioEditorExecutable->setText(QDir::toNativeSeparators(executable));
    mUI.edtAudioEditorExecutable->setCursorPosition(0);
}

void DlgSettings::on_btnSelectPython_clicked()
{
    QString filter;

#if defined(WINDOWS_OS)
    filter = "Python (python.exe)";
#else
    filter = "Python (python)";
#endif

    const QString& executable = QFileDialog::getOpenFileName(this,
        tr("Select Python Executable"), QString(), filter);
    if (executable.isEmpty())
        return;

    const QFileInfo info(executable);
    mUI.edtPythonExecutable->setText(QDir::toNativeSeparators(executable));
    mUI.edtPythonExecutable->setCursorPosition(0);
}
void DlgSettings::on_btnSelectEmsdk_clicked()
{
    const QString& dir = QFileDialog::getExistingDirectory(this,
        tr("Select Emsdk folder"), QString());
    if (dir.isEmpty())
        return;

    mUI.edtEmscriptenPath->setText(QDir::toNativeSeparators(dir));
    mUI.edtEmscriptenPath->setCursorPosition(0);
}

void DlgSettings::on_btnResetClearColor_clicked()
{
    constexpr QColor color = {0x23, 0x23, 0x23, 255};
    SetUIValue(mUI.clearColor, color);
}

void DlgSettings::on_btnResetGridColor_clicked()
{
    constexpr QColor color = {0xe3, 0xe3, 0xe3, 50};
    SetUIValue(mUI.gridColor, color);
}

void DlgSettings::on_editorFontName_currentIndexChanged(int)
{
    // fucking doesn't work, wtf, gives the previous font value.. wtf!?
    // have no interest to debug this now, so fucking hammer it!
    //UpdateSampleCode();

    QTimer::singleShot(0, this, [this]() {
        UpdateSampleCode();
    });

}
void DlgSettings::on_editorFontSize_currentIndexChanged(int)
{
    UpdateSampleCode();
}
void DlgSettings::on_editorHightlightSyntax_stateChanged(int)
{
    UpdateSampleCode();
}
void DlgSettings::on_editorHightlightCurrentLine_stateChanged(int)
{
    UpdateSampleCode();
}
void DlgSettings::on_editorShowLineNumbers_stateChanged(int)
{
    QTimer::singleShot(0, this, [this]() {
        UpdateSampleCode();
    });

    //UpdateSampleCode();
}
void DlgSettings::on_editorInsertSpaces_stateChanged(int)
{
    UpdateSampleCode();
}

} // namespace

