/**
 * Copyright (C) 2011 Mojmir Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#pragma once
#include <QThread>
#include <boost/circular_buffer.hpp>
#include <boost/tuple/tuple.hpp>
#include "cmd.h"
#include "logs/logwidget.h"
#include "plot/plotwidget.h"
#include "table/tablewidget.h"
#include "gantt/ganttdata.h"
#include "gantt/ganttwidget.h"
#include "gantt/frameview.h"
#include "dockedwidgets.h"
#include "appdata.h"
#include "constants.h"
#include "connectionconfig.h"
class MainWindow; class Server;
class QFile; class QDataStream; class QTextStream; class QTcpSocket;
class ControlBarCommon;

char const * const g_fileTags[] =  { g_LogTag , g_PlotTag , g_TableTag , g_GanttTag , g_FrameTag  };
char const * const g_fileNames[] = { g_LogFile, g_PlotFile, g_TableFile, g_GanttFile, g_FrameFile };

typedef DockedWidgets< logs::LogWidget,            logs::LogConfig 		> datalogs_t;
typedef DockedWidgets< plot::PlotWidget,           plot::PlotConfig     > dataplots_t;
typedef DockedWidgets< table::TableWidget,         table::TableConfig 	> datatables_t;
typedef DockedWidgets< gantt::GanttWidget,         gantt::GanttConfig   > datagantts_t;
typedef DockedWidgets< FrameView,                  FrameViewConfig      > dataframes_t;
typedef boost::tuple<datalogs_t, dataplots_t, datatables_t, datagantts_t, dataframes_t> data_widgets_t;

template <int TypeN>
struct SelectWidget { typedef typename boost::tuples::element<TypeN, data_widgets_t>::type::widget_t type; };
template <int TypeN>
struct SelectConfig { typedef typename boost::tuples::element<TypeN, data_widgets_t>::type::config_t type; };
template <int TypeN>
struct SelectIterator { typedef typename boost::tuples::element<TypeN, data_widgets_t>::type::iterator type; };


/**@class		Connection
 * @brief		represents incoming connection (or file stream)
 */
class Connection : public QThread, ActionAble
{
	Q_OBJECT
public:
	explicit Connection (QString const & app_name, QObject * parent = 0);
	~Connection ();

	MainWindow const * getMainWindow () const { return m_main_window; }
	MainWindow * getMainWindow () { return m_main_window; }
	GlobalConfig const & getGlobalConfig () const;
	
	QString const & getAppName () const { return m_app_name; }
	QString const & getCurrPreset () const { return m_curr_preset; }
	AppData const & appData () const { return m_app_data; }

	void run ();

	void saveConfigs (QString const & path);
	void loadConfigs (QString const & path);
	void loadConfig (QString const & preset_name);
	void saveConfig (QString const & preset_name);
	void applyConfigs ();

	//@TODO: old call!!
	void requestTableSynchronization (int sync_group, unsigned long long time);

	void requestTableWheelEventSync (int sync_group, QWheelEvent * ev, QTableView const * source);
	void requestTableActionSync (int sync_group, unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QTableView const * source);

	// data widget creation functions:
	template <int TypeN>
	bool dataWidgetConfigPreload (QString const tag, typename SelectConfig<TypeN>::type & config);
	QString getClosestPresetName ();
	E_FeatureStates getClosestFeatureState (E_DataWidgetType type) const;
	void mkWidgetPath (E_DataWidgetType type, QString const tag, QStringList & path);
	template <int TypeN>
	typename SelectIterator<TypeN>::type dataWidgetFactory (QString const tag);
	template <int TypeN>
	typename SelectIterator<TypeN>::type dataWidgetFactoryFrom (QString const tag, typename SelectConfig<TypeN>::type const & config);
	template <int TypeN>
	void removeDockedWidget (DockedWidgetBase * ptr);
	void destroyDockedWidget (DockedWidgetBase * dwb);

	// control widget
	void setPresetAsCurrent (QString const & pname);
	void mentionInPresetHistory (QString const & str);
	QString getCurrentPresetName () const;

signals:
	void readyForUse();
	void newMessage (QString const & from, QString const & message);
	void handleCommands ();
	
public slots:
	void onHandleCommands ();
	void onHandleCommandsStart ();
	void onHandleCommandsCommit ();
	bool tryHandleCommand (DecodedCommand const & cmd, E_ReceiveMode mode);

	// control widget
	void onLevelValueChanged (int i);
	void onBufferingStateChanged (int state);
	void onPresetChanged (int idx);
	void onPresetApply ();
	void onPresetSave ();
	void onPresetAdd ();
	void onPresetRm ();
	void onPresetReset ();
	void onLogsStateChanged (int state);
	void onPlotsStateChanged (int state);
	void onTablesStateChanged (int state);
	void onGanttsStateChanged (int state);
	void onPresetApply (QString const & preset_name);
	void onPresetSave (QString const & preset_name);
	void setLevelValue (int i);
	void setBufferingState (int state);
	void setLogsState (int state);
	void setPlotsState (int state);
	void setTablesState (int state);
	void setGanttsState (int state);

	void onSaveData (QString const & dir);
	void onExportDataToCSV (QString const & dir);

private slots:
	void processReadyRead ();
	void processTailCSVStream ();
	void handleCSVSetup (QString const & fname);
	void onDisconnected ();

protected:
	friend class Server;
	friend class MainWindow;
	friend struct DataLog;
	friend class gantt::GanttWidget;
	enum {
		e_data_ok = 0,
		e_data_pipe_full,
		e_data_decode_oor,
		e_data_decode_lnerr,
		e_data_decode_captain_failure,
		e_data_decode_general_failure,
		e_data_decode_error,
	};

	void setMainWindow (MainWindow * w) { m_main_window = w; }
	template <class T, typename T_Ret, typename T_Arg0, typename T_Arg1>
	int processStream (T *, T_Ret (T::*read_member_t)(T_Arg0, T_Arg1));
	bool enqueueCommand (DecodedCommand const & cmd);
	bool dequeueCommand (DecodedCommand & cmd);
	//void preparseCommand (DecodedCommand & cmd);

	bool handleSetupCommand (DecodedCommand const & cmd);
	bool handlePingCommand (DecodedCommand const & cmd);
	bool handleShutdownCommand (DecodedCommand const & cmd);
	bool handleDictionnaryCtx (DecodedCommand const & cmd);
	bool handleCSVStreamCommand (DecodedCommand const & cmd);
	bool handleExportCSVCommand (DecodedCommand const & cmd);
	bool handleSaveTLVCommand (DecodedCommand const & cmd);

	bool handleLogCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleLogClearCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleTableCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handlePlotCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleDataXYZCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleGanttBgnCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleGanttEndCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleGanttFrameBgnCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleGanttFrameEndCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleGanttClearCommand (DecodedCommand const & cmd, E_ReceiveMode mode);

	virtual bool handleAction (Action * a, E_ActionHandleType sync);
	virtual QWidget * controlWidget ();

	void registerDataMaps ();

	datalogs_t::iterator findOrCreateLog (QString const & tag);
	dataplots_t::iterator findOrCreatePlot (QString const & tag);
	datatables_t::iterator findOrCreateTable (QString const & tag);
	void appendTableXY (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & tag);
	void appendTableSetup (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & hhdr, QString const & tag);
	datagantts_t::iterator findOrCreateGantt (QString const & tag);
	void appendGantt (gantt::DecodedData &);
	dataframes_t::iterator findOrCreateFrame (QString const & tag);
	void appendFrames (gantt::DecodedData &);

	//void tryLoadMatchingPreset (QString const & appname);
	bool setupStorage (QString const & name);
	QString createStorageName () const;
	void processDataStream (QDataStream & stream);
	void copyStorageTo (QString const & filename);
	void closeStorage ();
	void setSocketDescriptor (int sd);
	void setImportFile (QString const & fname);
	void setTailFile (QString const & fname);

	void findTableIndexInFilters (QModelIndex const & row_index, bool scroll_to_item, bool expand);

	void setConfigValuesToUI (ConnectionConfig const & cfg);
	void setUIValuesToConfig (ConnectionConfig & cfg);
	void setupColumnSizes (bool force_setup = false);
	bool dumpModeEnabled () const;

private:
	QString m_app_name;
	MainWindow * m_main_window;
	E_SrcStream m_src_stream;
	E_SrcProtocol m_src_protocol;
	ConnectionConfig m_config;

	AppData m_app_data;
	QString m_pid;
	int m_storage_idx;
	unsigned m_recv_bytes;
	bool m_marked_for_close;
	QString m_curr_preset;
	ControlBarCommon * m_control_bar;
	QTextStream * m_file_csv_stream;
	QDataStream * m_file_tlv_stream;

	// data receiving stuff
	enum { e_ringbuff_size = 128 * 1024 };
	boost::circular_buffer<char> m_buffer;
	DecodedCommand m_current_cmd;
	enum { e_ringcmd_size = 16384 };
	boost::circular_buffer<DecodedCommand> m_decoded_cmds;
	QFile * m_storage;
	QDataStream * m_tcp_dump_stream;
	QTcpSocket * m_tcpstream;
	data_widgets_t m_data;
};

#include "connection.inl"

