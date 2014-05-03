package org.lance.filebrowe;

import org.lance.activity.R;

/** 表示一个文件实体 **/
public class FileInfo {
	private String fileName;
	private String filePath;
	private long fileSize;
	private boolean isDirectory;
	private int fileCount;
	private int folderCount;

	public String getFileName() {
		return fileName;
	}

	public void setFileName(String fileName) {
		this.fileName = fileName;
	}

	public String getFilePath() {
		return filePath;
	}

	public void setFilePath(String filePath) {
		this.filePath = filePath;
	}

	public long getFileSize() {
		return fileSize;
	}

	public void setFileSize(long fileSize) {
		this.fileSize = fileSize;
	}

	public boolean isDirectory() {
		return isDirectory;
	}

	public void setDirectory(boolean isDirectory) {
		this.isDirectory = isDirectory;
	}

	public int getFileCount() {
		return fileCount;
	}

	public void setFileCount(int fileCount) {
		this.fileCount = fileCount;
	}

	public int getFolderCount() {
		return folderCount;
	}

	public void setFolderCount(int folderCount) {
		this.folderCount = folderCount;
	}

	public int getIconResourceId() {
		if (isDirectory) {
			return R.drawable.file_folder;
		}
		return R.drawable.file_doc;
	}
}