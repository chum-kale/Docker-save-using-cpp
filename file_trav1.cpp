#include <dirent.h>
#include <vector>
#include <map>

using namespace std;

string layers_root= ("/var/snap/docker/common/var-lib-docker/image/overlay2/layerdb/sha256/");
string s = "diff";
map <string, string> layer_map;

void mappin(string path)
{
    DIR *dir;
    struct dirent *ent;
    string s = "diff";
    if ((dir = opendir (path)) != NULL)
    {
        new_path=(path + ent->d_name);
        if ((dir = opendir(new_path) != NULL))
        {
            if (s == ent->d_name)
            {
                final_path=("/var/snap/docker/common/var-lib-docker/image/overlay2/layerdb/sha256/" + ent->d_name);
                key = s.get();
                layer_map[key]=final_path;
            }
        }
    }
}