#include <iostream>
#include <dirent.h>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <fstream>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/json/value.hpp>
#include <boost/json/parse.hpp>
#include <string>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <algorithm>

using namespace boost::filesystem;
using namespace std;
namespace pt = boost::property_tree;

class docker
{
    string layers_root = ("/var/snap/docker/common/var-lib-docker/image/overlay2/layerdb/sha256/");
    string s = "diff";
    map<string, string> layer_map;

public:

    void load_layer_map();
    void save_image(string arg);
    void tar_split_decoder(string li, int lc);
    string b64_decode(string input);
};

void docker::load_layer_map()
{
    if (is_directory(layers_root))
    {
        DIR *dir;
        struct dirent *ent;
        dir = opendir(layers_root.c_str());
        while ((ent = readdir(dir)) != NULL)
        {
            string new_path = (layers_root + ent->d_name + "/");
            string diff_file = new_path + "diff";
            if (is_regular_file(diff_file))
            {
                std::ifstream ifs(diff_file); 
                std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
                layer_map[content] = new_path;
            }
        }
    }
}

void docker::save_image(string arg)
{
    int layer_count = 0;
    int layer_id;
    DIR *dr;
    struct dirent *fin;
    string image_path = ("/var/snap/docker/common/var-lib-docker/image/overlay2/imagedb/content/sha256/");
    image_path += arg;
    std::string data;
    pt::ptree root;
    std::vector<std::string> diff_ids;
    std::cout << data << std::endl;

    pt::read_json(image_path, root);

    for (pt::ptree::value_type &rootfs : root.get_child("rootfs"))
    {
        std::string id = rootfs.first;
        if (id == "diff_ids")
        {
            for (pt::ptree::value_type &value : rootfs.second)
            {
                diff_ids.push_back(value.second.get_value<std::string>());
            }
        }
    }
    std::map<string, string>::iterator itr = layer_map.begin();
    cout << "diff ids size: " << diff_ids.size() << endl;
    for (int i = 0; i < diff_ids.size(); i++)
    {
        cout << "for " << diff_ids[i] << endl;
        for (itr = layer_map.begin(); itr != layer_map.end(); itr++)
        {
            cout << "matching with: " << itr->first << endl;
            if (diff_ids[i] == itr->first)
            {
                cout << "Trying split decoding for " << itr->first << endl;
                tar_split_decoder(itr->second, i);
            }
        }
    }
}

void docker::tar_split_decoder(string li, int lc)
{
    string cache_id_path = li; 
    cache_id_path += "/cache-id";
    std::ifstream ifs(cache_id_path);
    std::string cache_id((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

    string tar_split_path = li; 
    tar_split_path += "/tar-split.json.gz";
    std::ifstream file(tar_split_path, ios_base::in | ios_base::binary);
    boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
    inbuf.push(boost::iostreams::gzip_decompressor());
    inbuf.push(file);
    istream instream(&inbuf);
    std::string line;
    std::string tf = std::to_string(lc) + ".tar";
    std::ofstream fil;
    fil.open (tf, ios::app | ios::binary);
    int bytes = 0;
    while (std::getline(instream, line))
    {
        std::istringstream iss(line);
        pt::ptree root;
        pt::read_json(iss, root);
        int type = root.get<int>("type", 0);
        string payload = root.get<string>("payload", "");
        if (type == 1 && !payload.empty() && payload != "null")
        {
            string cache_file_path = "/var/snap/docker/common/var-lib-docker/overlay2/";
            cache_file_path += cache_id;
            cache_file_path += "/diff/";
            string name = root.get<string>("name", "");
            cache_file_path = cache_file_path + name;
            std::ifstream ifs(cache_file_path);
            std::string cache_data((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
            fil.write(cache_data.data(), cache_data.size());
            bytes += cache_data.size();
        }
        if(type == 2 && !payload.empty() && payload != "null")
        {
            std::string op = b64_decode(payload);
            fil.write(op.data(), op.size());
            bytes += op.size();
        }
    }
    fil.close();
    cout << "Written bytes: " << bytes << endl;
}

string docker::b64_decode(string input)
{
            using namespace boost::archive::iterators;
            typedef transform_width<binary_from_base64<remove_whitespace
            <std::string::const_iterator> >, 8, 6> ItBinaryT;

            try
            {
                size_t num_pad_chars((4 - input.size() % 4) % 4);
                input.append(num_pad_chars, '=');

                size_t pad_chars(std::count(input.begin(), input.end(), '='));
                std::replace(input.begin(), input.end(), '=', 'A');
                std::string output(ItBinaryT(input.begin()), ItBinaryT(input.end()));
                output.erase(output.end() - pad_chars, output.end());
                return output;
            }
            catch (std::exception const& e)
            {
                std::cout << "exception: " << e.what() << std::endl;
                return std::string("");
            }
}



int main()
{
    docker d1;
    d1.load_layer_map();
    string ip;
    cout<<"Enter imagedb data:"<<endl;
    cin>>ip;
    d1.save_image(ip);
}