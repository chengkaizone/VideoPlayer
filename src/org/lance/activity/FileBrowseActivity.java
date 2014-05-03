package org.lance.activity;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.lance.adapter.FileAdapter;
import org.lance.decode.FFmpeg;
import org.lance.filebrowe.FileActivityHelper;
import org.lance.filebrowe.FileInfo;
import org.lance.filebrowe.FileUtil;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

/**
 * �ļ������ͼ
 * 
 * @author lance
 * 
 */
public class FileBrowseActivity extends ListActivity {
	private TextView _filePath;
	private List<FileInfo> _files;
	private String _rootPath = FileUtil.getSDPath();
	private String _currentPath = _rootPath;
	private final String TAG = "Main";
	private final int MENU_RENAME = Menu.FIRST;
	private final int MENU_COPY = Menu.FIRST + 3;
	private final int MENU_MOVE = Menu.FIRST + 4;
	private final int MENU_DELETE = Menu.FIRST + 5;
	private final int MENU_INFO = Menu.FIRST + 6;

	private int command = 0;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.browse_main);
		_filePath = (TextView) findViewById(R.id.file_path);
		// ��ȡ��ǰĿ¼���ļ��б�
		viewFiles(_currentPath);
		// ע�������Ĳ˵�
		registerForContextMenu(getListView());
		command = getIntent().getIntExtra("COMMAND", 1);
		Toast.makeText(FileBrowseActivity.this, "��ѡ��ý���ļ����в���~", 1).show();
	}

	/** �����Ĳ˵� **/
	@Override
	public void onCreateContextMenu(ContextMenu menu, View v,
			ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);

		AdapterView.AdapterContextMenuInfo info = null;

		try {
			info = (AdapterView.AdapterContextMenuInfo) menuInfo;
		} catch (ClassCastException e) {
			Log.e(TAG, "bad menuInfo", e);
			return;
		}

		FileInfo f = _files.get(info.position);
		menu.setHeaderTitle(f.getFileName());
		menu.add(0, MENU_RENAME, 1, getString(R.string.file_rename));
		menu.add(0, MENU_COPY, 2, getString(R.string.file_copy));
		menu.add(0, MENU_MOVE, 3, getString(R.string.file_move));
		menu.add(0, MENU_DELETE, 4, getString(R.string.file_delete));
		menu.add(0, MENU_INFO, 5, getString(R.string.file_info));
	}

	/** �����Ĳ˵��¼����� **/
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) item
				.getMenuInfo();
		FileInfo fileInfo = _files.get(info.position);
		File file = new File(fileInfo.getFilePath());
		switch (item.getItemId()) {
		case MENU_RENAME:
			FileActivityHelper.renameFile(FileBrowseActivity.this, file,
					renameFileHandler);
			return true;
		case MENU_COPY:
			pasteFile(file.getPath(), "COPY");
			return true;
		case MENU_MOVE:
			pasteFile(file.getPath(), "MOVE");
			return true;
		case MENU_DELETE:
			FileUtil.deleteFile(file);
			viewFiles(_currentPath);
			return true;
		case MENU_INFO:
			FileActivityHelper.viewFileInfo(this, file);
			return true;
		default:
			return super.onContextItemSelected(item);
		}
	}

	/** �б�����¼����� **/
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		FileInfo inf = _files.get(position);
		if (inf.isDirectory()) {
			viewFiles(inf.getFilePath());
		} else {
			// openFile(f.Path);//�������¶������ļ�����Ӧ�¼�
			String path = inf.getFilePath();
			if (!FFmpeg.getInstance().isMediaFile(path)) {
				Toast.makeText(this, "��ѡ��Ĳ���ý���ļ�", 1).show();
			} else {
				Intent data = new Intent(this, VideoPlayerActivity.class);
				data.putExtra("PATH", path);
				//startActivity(data);
			}
		}
	}

	/** �ض��巵�ؼ��¼� **/
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// ����back����
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			File f = new File(_currentPath);
			String parentPath = f.getParent();
			if (parentPath != null) {
				viewFiles(parentPath);
				return true;
			} else {
				// exit();
				return super.onKeyDown(keyCode, event);
			}
		}
		return super.onKeyDown(keyCode, event);
	}

	/** ��ȡ��PasteFile���ݹ�����·�� **/
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		if (Activity.RESULT_OK == resultCode) {
			Bundle bundle = data.getExtras();
			if (bundle != null && bundle.containsKey("CURRENTPATH")) {
				viewFiles(bundle.getString("CURRENTPATH"));
			}
		}
	}

	/** �����˵� **/
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = this.getMenuInflater();
		inflater.inflate(R.menu.browse_menu, menu);
		return true;
	}

	/** �˵��¼� **/
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.mainmenu_home:
			viewFiles(_rootPath);
			break;
		case R.id.mainmenu_refresh:
			viewFiles(_currentPath);
			break;
		case R.id.mainmenu_createdir:
			FileActivityHelper.createDir(FileBrowseActivity.this, _currentPath,
					createDirHandler);
			break;
		case R.id.mainmenu_exit:
			exit();
			break;
		default:
			break;
		}
		return true;
	}

	/** ��ȡ��Ŀ¼�������ļ� **/
	private void viewFiles(String filePath) {
		ArrayList<FileInfo> tmp = FileActivityHelper.getFiles(
				FileBrowseActivity.this, filePath);
		if (tmp != null) {
			if (_files != null) {
				_files.clear();
			}
			_files = tmp;
			_currentPath = filePath;
			_filePath.setText(filePath);
			setListAdapter(new FileAdapter(this, _files));
		}
	}

	/** �����¼����� **/
	/**
	 * private OnItemLongClickListener _onItemLongClickListener = new
	 * OnItemLongClickListener() {
	 * 
	 * @Override public boolean onItemLongClick(AdapterView<?> parent, View
	 *           view, int position, long id) { Log.e(TAG, "position:" +
	 *           position); return true; } };
	 **/

	/** ���ļ� **/
	private void openFile(String path) {
		Intent intent = new Intent();
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		intent.setAction(android.content.Intent.ACTION_VIEW);

		File f = new File(path);
		String type = FileUtil.getMIMEType(f.getName());
		intent.setDataAndType(Uri.fromFile(f), type);
		startActivity(intent);
	}

	/** �������ص�ί�� **/
	private final Handler renameFileHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			if (msg.what == 0)
				viewFiles(_currentPath);
		}
	};

	/** �����ļ��лص�ί�� **/
	private final Handler createDirHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			if (msg.what == 0)
				viewFiles(_currentPath);
		}
	};

	/** ճ���ļ� **/
	private void pasteFile(String path, String action) {
		Intent intent = new Intent();
		Bundle bundle = new Bundle();
		bundle.putString("CURRENTPASTEFILEPATH", path);
		bundle.putString("ACTION", action);
		intent.putExtras(bundle);
		intent.setClass(FileBrowseActivity.this, FilePasteActivity.class);
		// ��һ��Activity���ȴ����
		startActivityForResult(intent, 0);
	}

	/** �˳����� **/
	private void exit() {
		new AlertDialog.Builder(FileBrowseActivity.this)
				.setMessage(R.string.confirm_exit)
				.setCancelable(false)
				.setPositiveButton("Yes",
						new DialogInterface.OnClickListener() {
							@Override
							public void onClick(DialogInterface dialog,
									int which) {
								FileBrowseActivity.this.finish();
								android.os.Process
										.killProcess(android.os.Process.myPid());
								System.exit(0);
							}
						})
				.setNegativeButton("No", new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						dialog.cancel();
					}
				}).show();
	}
}
