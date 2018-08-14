#include "image_io.hpp"


void SearchDir(std::string path, std::vector<std::string> &output) {
    int i, dirElements;
    std::string searchPath;

    struct stat statBuf;
    struct dirent **nameList = NULL;

    if(*path.rbegin() != '/'){
        path.push_back('/');
    }

    // dirElements にはディレクトリ内の要素数が入る
    dirElements = scandir(path.c_str(), &nameList, NULL, NULL);

    if (dirElements == -1) {
        std::cout << "ERROR" << std::endl;
    } else {

        //ディレクトリかファイルかを順番に識別
        for (i = 0; i < dirElements; i += 1) {

            // "." と ".." を除く
            if ((strcmp(nameList[i]->d_name, ".\0") != 0) && (strcmp(nameList[i]->d_name, "..\0") != 0)) {

                //search_pathには検索対象のフルパスを格納する
                searchPath = path + std::string(nameList[i]->d_name);

                // ファイル情報の取得の成功
                if (stat(searchPath.c_str(), &statBuf) == 0) {

                    // ディレクトリだった場合の処理
                    if ((statBuf.st_mode & S_IFMT) == S_IFDIR) {
                        // 再帰によりディレクトリ内を探索
                        SearchDir(path + std::string(nameList[i]->d_name) + "/", output);
                    }

                        //ファイルだった場合の処理
                    else {
                        output.push_back(searchPath);
                    }
                }

                    // ファイル情報の取得の失敗
                else {
                    std::cout << "ERROR" << std::endl << std::endl;
                }
            }
        }
    }
    std::sort(output.begin(), output.end());
    free(nameList);
}
