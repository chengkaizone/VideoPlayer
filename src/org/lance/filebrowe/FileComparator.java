package org.lance.filebrowe;

import java.util.Comparator;

/** 排序 **/
public class FileComparator implements Comparator<FileInfo> {

	public int compare(FileInfo file1, FileInfo file2) {
		// 文件夹排在前面
		if (file1.isDirectory() && !file2.isDirectory()) {
			return -1000;
		} else if (!file1.isDirectory() && file2.isDirectory()) {
			return 1000;
		}
		// 相同类型按名称排序
		return file1.getFileName().compareTo(file2.getFileName());
	}
}