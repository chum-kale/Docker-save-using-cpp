#include <iostream>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;

int load_layer_map(char *image_dir)
{
    // check if the dir exists
    struct stat sb;
    if (NULL == image_dir || stat(image_dir, &sb) != 0) {
        cout << "Image dir doesn't exist" << endl;
        return -1;
    }

    // get a list of all images
    DIR *dr;
    struct dirent *en;
    dr = opendir(image_dir);
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            cout <<" \n" << en->d_name;
            // if it's a dir
            // open dir/diff file
            // read contents of diff
            // use it as a key in a map structure
            // store path as value
        }
    }
    closedir(dr);

    return 0;
}

const string docker_image_dir = "/var/snap/docker/common/var-lib-docker/image/overlay2/layerdb/sha256/";
int main(int argc, char **argv)
{
    int ret=0;

    ret = load_layer_map(argv[1]);

    return ret;
}
