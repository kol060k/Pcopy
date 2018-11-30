#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <string>
#include <vector>
#include <utime.h>
#include <fcntl.h>

using namespace std;

struct copy_info {
	string from;
	string to;
};

struct pthread_data {
	pthread_mutex_t* mutex;
	vector<copy_info>* file_list;
	int me;
};

void copy_dir(string source, string receiver, vector<copy_info>* file_list) {
// Функция копирования дерева директорий из истоника в приёмник с добавлением найденных файлов в file_list
	DIR *ds = opendir(source.c_str());
	DIR *dr = opendir(receiver.c_str());
	for (dirent *de = readdir(ds); de != NULL; de = readdir(ds)) {
		string new_source = source + "/" + de->d_name;
		string new_receiver = receiver + "/" + de->d_name;
		if (de->d_type == DT_DIR) {
			if (strcmp(de->d_name, ".") == 0) continue;
			if (strcmp(de->d_name, "..") == 0) continue;
			struct stat st;
			if (stat(new_receiver.c_str(), &st) == -1) {
				struct stat st1;
				stat(new_source.c_str(), &st1);
				mkdir(new_receiver.c_str(), st1.st_mode % 512);
			}
			copy_dir(new_source, new_receiver, file_list);
		}
		else {
			copy_info new_file;
			new_file.from = new_source;
			new_file.to = new_receiver;
			file_list->push_back(new_file);
		}
	}
	closedir(ds);
	closedir(dr);
}

void* file_copy(void* data_) {			// Функция копирования файлов
	pthread_data* data = (pthread_data*) data_;
	while(1) {
		pthread_mutex_lock(data->mutex);	// Забираем файл из списка файлов
		if (data->file_list->empty()){
			pthread_mutex_unlock(data->mutex);
			return NULL;
		}
		string from = (*data->file_list)[0].from;
		string to = (*data->file_list)[0].to;
		data->file_list->erase(data->file_list->begin());
		pthread_mutex_unlock(data->mutex);
		
		struct stat st;
		if (stat(to.c_str(), &st) != -1) {	// Перименование старого файла
			string old = to + ".old";
			rename (to.c_str(), old.c_str());
		}
		stat(from.c_str(), &st);

		int fin = open(from.c_str(), O_RDONLY);		// Копирование
		if (fin < 0) {
			perror(from.c_str());
			return NULL;
		}
		int fout = open(to.c_str(), O_WRONLY | O_CREAT, st.st_mode);
		if (fout < 0) {
			perror(to.c_str());
			close(fin);
			return NULL;
		}
		int buffer_len = 65536;
		char buffer[buffer_len];
		int rd;
		while ((rd = read(fin, buffer, buffer_len)) > 0) {
			write(fout, buffer, rd);
		}
		close(fin);
		close(fout);
		struct utimbuf new_time;
		new_time.modtime = st.st_mtime;
		new_time.actime = st.st_atime;
		utime(to.c_str(), &new_time);
		cout << from << " -> " << to << " - DONE" << " by " << data->me << endl;
	}
}

int main(int argc, char** argv) {
	if (argc != 4) {			// Банальная проверка правильности ввода команды
		cout << "Wrong attributes" << endl;
		return 1;
	}
	int thread_number = atoi(argv[1]);	// Фиксируем количество thread'ов, с помощью которого будем копировать файлы

//// Создадим дерево директорий как в директории-источнике
	vector<copy_info> file_list;
	DIR *d = opendir(argv[2]);
	if (d == NULL) {
		perror(argv[2]);
		return 1;
	}
	closedir(d);
	d = opendir(argv[3]);
	if (d == NULL) {
		perror(argv[3]);
		return 1;
	}
	closedir(d);
	string source = argv[2];
	string receiver = argv[3];
	copy_dir(source, receiver, &file_list);

//// Приступаем к копированию файлов
	pthread_data data[thread_number];
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);
	for (int i = 0; i < thread_number; i++) {
		data[i].file_list = &file_list;
		data[i].mutex = &mutex;
		data[i].me = i;
	}
	pthread_t threads[thread_number];
	for (int i = 0; i < thread_number; i++) {
		pthread_create(&threads[i], NULL, file_copy, &data[i]);
	}
	for (int i = 0; i < thread_number; i++) {
		pthread_join(threads[i], NULL);
	}
	return 0;
}
