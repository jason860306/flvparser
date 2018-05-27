// This app splits an FLV file based on cue points located in a data file.   
// It also separates the video from the audio.   
// All the reverse_bytes action stems from the file format being big-endian   
// and needing an integer to hold a little-endian version (for proper calculation).   

#include "stdafx.h"

#pragma pack(1)

#define FLV_HEADER_SIGNATURE "FLV"
#define FLV_OBJECT_END_MARKER "009"

//************ dump type
#define DUMP_TYPE_DEFAULT 0
#define DUMP_TYPE_XML 1

//************ Constants
#define TAG_TYPE_AUDIO 8
#define TAG_TYPE_VIDEO 9
#define TAG_TYPE_META 18
#define CUE_BLOCK_SIZE 32
#define FLAG_SEPARATE_AV 1

//*********** amf type define
#define AMF_TYPE_NUMBER          0
#define AMF_TYPE_BOOLEAN         1
#define AMF_TYPE_STRING          2
#define AMF_TYPE_OBJECT          3
#define AMF_TYPE_MOVIECLIP       4
#define AMF_TYPE_NULL            5
#define AMF_TYPE_UNDEFINED       6
#define AMF_TYPE_REFERENCE       7
#define AMF_TYPE_ECMA_ARRAY      8
#define AMF_TYPE_OBJECT_END      9
#define AMF_TYPE_STRICT_ARRAY   10
#define AMF_TYPE_DATE           11
#define AMF_TYPE_LONG_STRING    12

//*********** sound format define
#define FLV_AUDIO_TAG_SOUND_FORMAT_LINEAR_PCM          0
#define FLV_AUDIO_TAG_SOUND_FORMAT_ADPCM               1
#define FLV_AUDIO_TAG_SOUND_FORMAT_MP3                 2
#define FLV_AUDIO_TAG_SOUND_FORMAT_LINEAR_PCM_LE       3
#define FLV_AUDIO_TAG_SOUND_FORMAT_NELLYMOSER_16_MONO  4
#define FLV_AUDIO_TAG_SOUND_FORMAT_NELLYMOSER_8_MONO   5
#define FLV_AUDIO_TAG_SOUND_FORMAT_NELLYMOSER          6
#define FLV_AUDIO_TAG_SOUND_FORMAT_G711_A              7
#define FLV_AUDIO_TAG_SOUND_FORMAT_G711_MU             8
#define FLV_AUDIO_TAG_SOUND_FORMAT_RESERVED            9
#define FLV_AUDIO_TAG_SOUND_FORMAT_AAC                 10
#define FLV_AUDIO_TAG_SOUND_FORMAT_SPEEX               11
#define FLV_AUDIO_TAG_SOUND_FORMAT_MP3_8               14
#define FLV_AUDIO_TAG_SOUND_FORMAT_DEVICE_SPECIFIC     15

//*********** sound rate define
#define FLV_AUDIO_TAG_SOUND_RATE_5_5     0
#define FLV_AUDIO_TAG_SOUND_RATE_11      1
#define FLV_AUDIO_TAG_SOUND_RATE_22      2
#define FLV_AUDIO_TAG_SOUND_RATE_44      3

//*********** mono or sterno sound define
#define FLV_AUDIO_TAG_SOUND_TYPE_MONO    0
#define FLV_AUDIO_TAG_SOUND_TYPE_STEREO  1

//*********** sound sample size define
#define FLV_AUDIO_TAG_SOUND_SIZE_8       0
#define FLV_AUDIO_TAG_SOUND_SIZE_16      1

//*********** video's codec define
#define FLV_VIDEO_TAG_CODEC_JPEG            1
#define FLV_VIDEO_TAG_CODEC_SORENSEN_H263   2
#define FLV_VIDEO_TAG_CODEC_SCREEN_VIDEO    3
#define FLV_VIDEO_TAG_CODEC_ON2_VP6         4
#define FLV_VIDEO_TAG_CODEC_ON2_VP6_ALPHA   5
#define FLV_VIDEO_TAG_CODEC_SCREEN_VIDEO_V2 6
#define FLV_VIDEO_TAG_CODEC_AVC             7

//*********** video's frame type define
#define FLV_VIDEO_TAG_FRAME_TYPE_KEYFRAME               1
#define FLV_VIDEO_TAG_FRAME_TYPE_INTERFRAME             2
#define FLV_VIDEO_TAG_FRAME_TYPE_DISPOSABLE_INTERFRAME  3
#define FLV_VIDEO_TAG_FRAME_TYPE_GENERATED_KEYFRAME     4
#define FLV_VIDEO_TAG_FRAME_TYPE_COMMAND_FRAME          5

//*********** TYPEDEFs
typedef uint8_t uint24_t[3];
typedef double amf_number_t;
typedef struct __amf_data_value amf_data_value_t;
typedef std::list<amf_data_value_t *> amf_script_data_list_t;
typedef struct __amf_object_property amf_object_property_t;
typedef std::list<amf_object_property_t *> amf_obj_property_list_t;

typedef struct __flv_hdr {
    uint8_t signature[3];
    uint8_t version;
    uint8_t flags;
    uint32_t data_offset;
} flv_hdr_t;

typedef struct __flv_tag {
    uint8_t tag_type;
    uint24_t data_size;
    uint24_t timestamp;
    uint8_t timestampex;
    uint24_t reserved;
} flv_tag_t;

typedef struct __amf_string {
    uint16_t size;
    uint8_t *data;
} amf_string_t;

typedef struct __amf_long_string {
    uint32_t size;
    uint8_t *data;
} amf_long_string_t;

typedef struct __amf_date {
    amf_number_t date_time;
    int16_t offset;
} amf_date_t;

typedef struct __amf_object_property {
    amf_string_t property_name;
    amf_data_value_t *p_data_value;
} amf_object_property_t;

typedef struct __amf_object_end_marker {
    uint24_t end_mark;
} amf_object_end_marker_t;

typedef struct __amf_emca_array {
    uint32_t arr_len;
    amf_obj_property_list_t object_property_lst;
    amf_object_end_marker_t object_end_marker;
} amf_emca_array_t;

typedef struct __amf_object {
    amf_obj_property_list_t object_property_lst;
    amf_object_end_marker_t object_end_marker;
} amf_object_t;

typedef struct __amf_strict_array {
    uint32_t arr_len;
    amf_script_data_list_t amf_data_value_lst;
} amf_strict_array_t;

typedef struct __amf_data_value {
    uint8_t type;
    typedef struct __data_value {
        amf_number_t number;
        uint8_t boolean_vaule;
        uint16_t reference_number;
        amf_emca_array_t *p_emca_array;
        amf_strict_array_t *p_strict_array;
        amf_object_t *p_object;
        amf_date_t date_value;
        amf_string_t string_value;
        amf_long_string_t long_string_value;
    } data_value_t;
    data_value_t data_value;
} amf_data_value_t;

typedef struct __flv_body {
    uint32_t pre_tag_size;
    flv_tag_t flv_tag;
    typedef struct __flv_body_data {
        uint8_t audio_video_hdr;
        amf_script_data_list_t amf_script_data_lst;
    } flv_body_data_t;
    flv_body_data_t flv_body_data;
} flv_body_t;

typedef struct __flv_file {
    flv_hdr_t flv_hdr;
    std::list<flv_body_t> flv_body_lst;
} flv_file_t;

//********* global variables
uint32_t g_cur_num = 0, g_flags = 0;
char g_project_name[_MAX_PATH];
flv_file_t g_flv_file;

//********* audio's info define
static const char *audio_format_info[] = {
    "Linear PCM, platform endian",
    "ADPCM",
    "MP3",
    "Linear PCM, little endian",
    "Nellymoser 16 kHz mono",
    "Nellymoser 8 kHz mono",
    "Nellymoser",
    "G.711 A-law logarithmic PCM",
    "G.711 mu-law logarithmic PCM",
    "reserved",
    "AAC",
    "Speex",
    "MP3 8 kHz",
    "Device-specific sound"
};

static const char *audio_rate_info[] = {
    "5.5 kHz",
    "11 kHz",
    "22 kHz",
    "44 kHz"
};

static const char *audio_mono_streno_info[] = {
    "Mono sound",
    "Stereo sound"
};

static const char *audio_sample_size_info[] = {
    "8-bit samples",
    "16-bit samples"
};

//********* video's info define
static const char *video_codec_info[] = {
    "JPEG",
    "Sorenson H.263",
    "Screen video",
    "On2 VP6",
    "On2 VP6 with alpha channel",
    "Screen video version 2",
    "AVC"
};

static const char *video_frame_type[] = {
    "key frame (for AVC, a seekable frame)",
    "inter frame (for AVC, a non-seekable frame)",
    "disposable inter frame (H.263 only)",
    "generated key frame (reserved for server use only)",
    "video info/command frame"
};

//********** local function prototypes
uint32_t copymem(char *destination, char *source, uint32_t byte_count);
uint32_t fget(FILE *filehandle, char *buffer, uint32_t buffer_size);
uint32_t fput(FILE *filehandle, char *buffer, uint32_t buffer_size);
FILE *open_output_file(uint8_t tag_type);
void processfile(char *flv_filename, char *cue_file);
uint32_t *read_cue_file(char *cue_file_name);
uint32_t xfer(FILE *input_file_handle, FILE *output_file_handle, uint32_t byte_count);

//********** functions for amf's object
amf_number_t read_number(FILE *ifh, amf_number_t **pp_amf_number);
uint8_t read_byte(FILE *ifh, uint8_t *pp_amf_byte);
const uint8_t *read_string(FILE *ifh, amf_string_t *pp_amf_string);
const uint8_t *read_long_string(FILE *ifh, amf_long_string_t *pp_amf_long_string);
const uint8_t *read_datadate(FILE *ifh, amf_date_t *pp_amf_datadate);
uint32_t read_object(FILE *ifh, FILE *parse_file, amf_object_t **pp_amf_object);
uint32_t read_emca_array(FILE *ifh, FILE *parse_file, amf_emca_array_t **pp_amf_emca_array);
uint32_t read_strict_array(FILE *ifh, FILE *parse_file, amf_strict_array_t **pp_amf_strict_array);
const uint8_t *read_end_marker(FILE *ifh, amf_data_value_t *p_amf_marker);
uint8_t read_amf_data(FILE *ifh, FILE *parse_file, amf_data_value_t **pp_amf_data);

//********** free functions for amf's object
void free_amf_obj_property(amf_object_property_t *p_obj_property);
void free_amf_data(amf_data_value_t *p_data_value);

//********** dump functions for amf's object
void dump_flv_file();
void dump_meta_data(amf_data_value_t *p_data_value, FILE *xml_file);

//Defines the entry point for the console application.
#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    if (argc < 3) {
        printf("usage: %s flv_file cue [ --split ]\n", argv[0]);
        printf("  cue_file - a file store some cue time point.\n");
        printf("             e.g. : \n");
        printf("             00:11:14:00\n");
        printf("             02:05:04:38\n");
        printf("             03:12:14:21\n");
        printf("             03:04:14:13\n");
        printf("             04:13:15:23\n");
        printf("  split    - split audio and video into a stand-alone file\n");
        exit(EXIT_FAILURE);
    }
    else {
        if ((argc==4) && (strstr(argv[3],"--split")!=NULL)) {
            g_flags |= FLAG_SEPARATE_AV;
        }
        //printf("sizeof(flv_hdr_t) = %d\n", sizeof(flv_hdr_t));
        //printf("sizeof(flv_tag_t) = %d\n", sizeof(flv_tag_t));
        processfile(argv[1], argv[2]);
    }
    //getchar();

    return 0;
}

//processfile is the central function   
void processfile(char *in_file, char *cue_file){   

    FILE *ifh=NULL, *vfh=NULL, *afh = NULL, *parse_file = NULL;
    flv_hdr_t &flv_hdr = g_flv_file.flv_hdr;
    flv_body_t flv_body;
    flv_tag_t &flv_tag = flv_body.flv_tag;
    uint32_t &pre_tag_size = flv_body.pre_tag_size, pts_z=0;
    uint32_t *cue, ts = 0, ts_new = 0, ts_offset = 0;
    uint32_t ptag = DUMP_TYPE_DEFAULT, timestamp = 0, datasize = 0;

    //set project name
    strncpy(g_project_name, in_file, strstr(in_file, ".flv") - in_file);

    //open the input file   
    if ((ifh = fopen(in_file, "rb")) == NULL) {   
        fprintf(parse_file, "Failed to open %s", in_file);   
        return;   
    }
    if ((parse_file = open_output_file(ptag)) == NULL) {
        return;
    }

    fprintf(parse_file, "Processing [%s] with cue file [%s]\n", in_file, cue_file);

    //build cue array   
    cue = read_cue_file(cue_file);   

    //capture the FLV file header   
    fget(ifh, (char *)&flv_hdr, sizeof(flv_hdr_t));

    //move the file pointer to the end of the header
    std::reverse((uint8_t *)&flv_hdr.data_offset, (uint8_t *)&flv_hdr.data_offset + sizeof(flv_hdr.data_offset));
    datasize = flv_hdr.data_offset;
    fseek(ifh, datasize, SEEK_SET);

    fprintf(parse_file, "================= flv.header(: %lu) =====================\n", sizeof(flv_hdr_t));
    fprintf(parse_file, "flv.header.signature[3] = '%c' '%c' '%c'\n", flv_hdr.signature[0], flv_hdr.signature[1], flv_hdr.signature[2]);
    fprintf(parse_file, "flv.header.version = 0x%X\n", flv_hdr.version);
    fprintf(parse_file, "flv.header.flags = 0x%X\n", flv_hdr.flags);
    fprintf(parse_file, "flv.header.flags.has_audio = %d\n", (flv_hdr.flags & 0x04) != 0);
    fprintf(parse_file, "flv.header.flags.has_video = %d\n", (flv_hdr.flags & 0x01) != 0);
    fprintf(parse_file, "flv.header.dataoffset = %u\n", datasize);

    fprintf(parse_file, "\n================= flv.tag =====================\n");
    //process each tag in the file   
    do {   

        //capture the PreviousTagSize integer   
#ifdef _WIN32
        pre_tag_size = _getw(ifh);
#else
	pre_tag_size = getw(ifh);
#endif
        std::reverse((uint8_t *)&pre_tag_size, (uint8_t *)&pre_tag_size + sizeof(pre_tag_size));
        fprintf(parse_file, "pre_tag_size:   %d\n", pre_tag_size);

        //extract the tag from the input file   
        fget(ifh, (char *)&flv_tag, sizeof(flv_tag_t));   

        //set the tag value to select on   
        ptag = flv_tag.tag_type;   

        //if we are not separating AV, process the audio like video   
        if (!(g_flags && FLAG_SEPARATE_AV)) {
            ptag = TAG_TYPE_VIDEO;
        }

        //if we are not past the end of file, process the tag   
        if (!feof(ifh)) {

            //if we've exceed the cuepoint then close output files and select next cuepoint
            std::reverse((uint8_t *)&flv_tag.timestamp, (uint8_t *)&flv_tag.timestamp + sizeof(flv_tag.timestamp));
            memcpy(&timestamp, flv_tag.timestamp, sizeof(flv_tag.timestamp));
            timestamp += flv_tag.timestampex << 24;
            std::reverse((uint8_t *)&flv_tag.data_size, (uint8_t *)&flv_tag.data_size + sizeof(flv_tag.data_size));
            memcpy(&datasize, flv_tag.data_size, sizeof(flv_tag.data_size));

            fprintf(parse_file, "\n================= flv.tag.head(: %lu) =====================\n", sizeof(flv_tag_t));
            fprintf(parse_file, "flv.tag.tagType     = %d\n", ptag);
            fprintf(parse_file, "flv.tag.datasize    = %d\n", datasize);
            fprintf(parse_file, "flv.tag.Timestamp   = %d\n", timestamp);
            fprintf(parse_file, "flv.tag.TimestampEx = %d", flv_tag.timestampex);

            if (timestamp > cue[g_cur_num]) {

                //close any audio file and designated closed with NULL   
                if (afh != NULL) {
                    fclose(afh);
                    afh = NULL;
                }

                //close any video file and designated closed with NULL   
                if (vfh != NULL) {
                    fclose(vfh);
                    vfh = NULL;
                }

                //increment the current slide   
                g_cur_num++;   

                //provide feedback to the user   
                fprintf(parse_file, "Processing slide %i...\n", g_cur_num);   
            }   

            //process tag by type   
            switch (ptag) {   

            case TAG_TYPE_AUDIO:  //we only process like this if we are separating audio into an mp3 file   
                {
                    fprintf(parse_file, "\n================= flv.tag.body.audio.header =====================\n");
                    //if the output file hasn't been opened, open it.   
                    if (afh == NULL) {
                        if ((afh = open_output_file(ptag)) == NULL)
                        {
                            fprintf(parse_file, "open file fail, err = %s\n",
				strerror(errno));
                            break;
                        }
                    }
#if 0
                    //jump past audio tag header uint8_t
                    fseek(ifh, 1, SEEK_CUR);
#endif
                    uint8_t &flv_audio_header = flv_body.flv_body_data.audio_video_hdr;
                    fget(ifh, (char *)&flv_audio_header, sizeof(flv_audio_header));
                    // decoce audio tag header.
                    short sound_format = (flv_audio_header >> 4) & 0x0F;
                    short sample_rate = (flv_audio_header >> 2) & 0x03;
                    short sample_size = (flv_audio_header >> 1) & 0x01;
                    short sound_type = (flv_audio_header >> 0) & 0x01;
                    fprintf(parse_file, "sound format: %2d - %s\n", sound_format, audio_format_info[sound_format]);
                    fprintf(parse_file, "sound rate:   %2d - %s\n", sample_rate, audio_rate_info[sample_rate]);
                    fprintf(parse_file, "sample size:  %2d - %s\n", sample_size, audio_sample_size_info[sample_size]);
                    fprintf(parse_file, "sound type:   %2d - %s\n", sound_type, audio_mono_streno_info[sound_type]);
                    fprintf(parse_file, "datasize:     %d\n", datasize);
#if 0
                    if (sound_format == FLV_AUDIO_TAG_SOUND_FORMAT_AAC)
                    {
                        uint8_t aac_pkt_type = 0x0;
                        fget(ifh, (char *)&aac_pkt_type, sizeof(aac_pkt_type));
                        aac_pkt_type = reverse_bytes(&aac_pkt_type, sizeof(aac_pkt_type));
                        fprintf(parse_file, "AAC Packet Type: %d - %s\n", aac_pkt_type, aac_pkt_type ? "AAC raw" : "AAC sequence header");
                    }
#endif

                    //dump the audio data to the output file
                    xfer(ifh, afh, datasize - 1);
                }

                break;   

            case TAG_TYPE_VIDEO:
                {
                    fprintf(parse_file, "\n================= flv.tag.body.video.header =====================\n");
                    //if the output file hasn't been opened, open it.   
                    if (vfh == NULL) {   

                        //get the new video output file pointer   
                        vfh = open_output_file(ptag);   

                        //record the timestamp offset for this slice
                        std::reverse((uint8_t *)&flv_tag.timestamp, (uint8_t *)&flv_tag.timestamp + sizeof(flv_tag.timestamp));
                        memcpy(&ts_offset, flv_tag.timestamp, sizeof(flv_tag.timestamp));

                        //write the flv header (reuse the original file's hdr) and first pts   
                        fput(vfh, (char *)&flv_hdr, sizeof(flv_hdr));   
                        fput(vfh, (char *)&pts_z, sizeof(pts_z));   
                    }   

                    //offset the timestamp in the tag   
                    std::reverse((uint8_t *)&flv_tag.timestamp, (uint8_t *)&flv_tag.timestamp + sizeof(flv_tag.timestamp));
                    memcpy(&timestamp, flv_tag.timestamp, sizeof(flv_tag.timestamp));
                    ts = timestamp - ts_offset;

                    //reverse the timestamp bytes back into BigEndian
                    std::reverse((uint8_t *)&ts, (uint8_t *)&ts + sizeof(ts));
                    ts_new = ts;

                    //overwrite the highest 3 bytes of the integer into the timestamp   
                    copymem((char *)&flv_tag.timestamp, (char *)(((char *)&ts_new) + 1), sizeof(flv_tag.timestamp));   

                    //write tag to output file   
                    fput(vfh, (char *)&flv_tag, sizeof(flv_tag));

                    // decode video's header
                    uint8_t &flv_video_header = flv_body.flv_body_data.audio_video_hdr;
                    fget(ifh, (char *)&flv_video_header, sizeof(flv_video_header));
                    short frame_type = (flv_video_header >> 4) & 0x0F;
                    short codec_id = (flv_video_header >> 0) & 0x0F;
                    fprintf(parse_file, "frame type: %3d - %s\n", frame_type, video_frame_type[frame_type - 1]);
                    fprintf(parse_file, "codec id:   %3d - %s\n", codec_id, video_codec_info[codec_id - 1]);
                    fprintf(parse_file, "datasize:     %d\n", datasize);
                    fseek(ifh, -1, SEEK_CUR);

                    //dump the video data to the output file, including the PTS field
                    xfer(ifh, vfh, datasize + 4);

                    //rewind 4 bytes, because we need to read the PTS again for the loop's sake   
                    fseek(ifh, -4, SEEK_CUR);
                }

                break;

            case TAG_TYPE_META:
                fprintf(parse_file, "\n================= flv.tag.event(onMetaData).header =====================");
                {
                    long fpos = ftell(ifh);
                    {
                        amf_data_value_t *p_amf_data = new amf_data_value_t();
                        assert(read_byte(ifh, &p_amf_data->type) == AMF_TYPE_STRING);
                        read_string(ifh, &p_amf_data->data_value.string_value);
                        flv_body.flv_body_data.amf_script_data_lst.push_back(p_amf_data);

                        p_amf_data = new amf_data_value_t();
                        read_amf_data(ifh, parse_file, &p_amf_data);
                        flv_body.flv_body_data.amf_script_data_lst.push_back(p_amf_data);
                    }
                    fseek(ifh, fpos, SEEK_SET);
                }
            default:
                //skip the data of this tag
                fseek(ifh, datasize, SEEK_CUR);
            }
        }

        g_flv_file.flv_body_lst.push_back(flv_body);

    } while (!feof(ifh));

    dump_flv_file();

    std::for_each(flv_body.flv_body_data.amf_script_data_lst.begin(), flv_body.flv_body_data.amf_script_data_lst.end(), &free_amf_data);

#ifdef _WIN32
    //finished...close all file pointers   
    _fcloseall();
#else
	fcloseall();
#endif

    //feedback to user   
    fprintf(parse_file, "Program complete.");
}

uint8_t read_byte(FILE *ifh, uint8_t *p_amf_byte)
{
    if (NULL == p_amf_byte)
    {
        p_amf_byte = new uint8_t();
    }
    uint8_t &cn = *p_amf_byte;
    fget(ifh, (char*)&cn, sizeof(cn));
    std::reverse((uint8_t *)&cn, (uint8_t *)&cn + sizeof(cn));

    return cn;
}

amf_number_t read_number(FILE *ifh, amf_number_t *p_amf_number)
{
    if (NULL == p_amf_number)
    {
        p_amf_number = new amf_number_t();
    }
    amf_number_t &dn = *p_amf_number;
    fget(ifh, (char*)&dn, sizeof(dn));
    std::reverse((uint8_t *)&dn, (uint8_t *)&dn + sizeof(dn));

    return dn;
}

const uint8_t *read_string(FILE *ifh, amf_string_t *p_amf_string)
{
    if (NULL == p_amf_string)
    {
        p_amf_string = new amf_string_t();
    }
    uint16_t &usn = p_amf_string->size;
    uint8_t *&data = p_amf_string->data;
    fget(ifh, (char*)&usn, sizeof(usn));
    std::reverse((uint8_t *)&usn, (uint8_t *)&usn + sizeof(usn));
    data = new uint8_t[usn + 1];
    data[usn] = '\0';
    fget(ifh, (char*)data, usn);

    return data;
}

const uint8_t *read_long_string(FILE *ifh, amf_long_string_t *p_amf_long_string)
{
    if (NULL == p_amf_long_string)
    {
        p_amf_long_string = new amf_long_string_t();
    }
    uint32_t &usn = p_amf_long_string->size;
    uint8_t *&data = p_amf_long_string->data;
    fget(ifh, (char*)&usn, sizeof(usn));
    std::reverse((uint8_t *)&usn, (uint8_t *)&usn + sizeof(usn));
    data = new uint8_t[usn + 1];
    data[usn] = '\0';
    fget(ifh, (char*)data, usn);

    return data;
}

const uint8_t *read_datadate(FILE *ifh, amf_date_t *p_amf_datetime)
{
    if (NULL == p_amf_datetime)
    {
        p_amf_datetime = new amf_date_t();
    }
    amf_number_t &date_time = p_amf_datetime->date_time;
    int16_t &offset = p_amf_datetime->offset;
    read_number(ifh, &date_time);
    fget(ifh, (char *)&offset, sizeof(offset));
    std::reverse((uint8_t *)&offset, (uint8_t *)&offset + sizeof(offset));

    static uint8_t local_time[] = { 0 };
    return local_time;
}

uint32_t read_object(FILE *ifh, FILE *parse_file, amf_object_t **pp_amf_object)
{
    if (NULL == *pp_amf_object)
    {
        *pp_amf_object = new amf_object_t();
    }
    amf_object_t *p_amf_object = *pp_amf_object;
    uint32_t arr_size = 0;
    fprintf(parse_file, "object:\n");
    while (true)
    {
        amf_object_property_t *p_object_property = new amf_object_property_t();
        amf_string_t *p_amf_string = &p_object_property->property_name;
        const uint8_t *name = read_string(ifh, p_amf_string);
        if (strlen((const char *)name))
        {
            fprintf(parse_file, "\t%s: ", name);
        }

        uint8_t type = read_amf_data(ifh, parse_file, &p_object_property->p_data_value);
        p_amf_object->object_property_lst.push_back(p_object_property);
        if (type == AMF_TYPE_OBJECT_END)
        {
            break;
        }
        ++arr_size;
    }

    return arr_size;
}

uint32_t read_emca_array(FILE *ifh, FILE *parse_file, amf_emca_array_t **pp_amf_emca_array)
{
    if (NULL == *pp_amf_emca_array)
    {
        *pp_amf_emca_array = new amf_emca_array_t();
    }
    amf_emca_array_t *p_amf_emca_array = *pp_amf_emca_array;
    uint32_t &arr_size = p_amf_emca_array->arr_len;
    fget(ifh, (char*)&arr_size, sizeof(arr_size));
    std::reverse((uint8_t *)&arr_size, (uint8_t *)&arr_size + sizeof(arr_size));
    
    fprintf(parse_file, "emca_array:\n");
    for (uint32_t i = 0; i < arr_size; ++i)
    {
        amf_object_property_t *p_amf_obj_property = new amf_object_property_t();
        p_amf_obj_property->p_data_value = new amf_data_value_t();
        amf_string_t *p_amf_string = &p_amf_obj_property->property_name;
        fprintf(parse_file, "\t%s: ", read_string(ifh, p_amf_string));

        uint8_t type = read_amf_data(ifh, parse_file, &p_amf_obj_property->p_data_value);
        p_amf_emca_array->object_property_lst.push_back(p_amf_obj_property);
        if (type == AMF_TYPE_OBJECT_END)
        {
            break;
        }
    }
    return arr_size;
}

uint32_t read_strict_array(FILE *ifh, FILE *parse_file, amf_strict_array_t **pp_amf_strict_array)
{
    if (NULL == *pp_amf_strict_array)
    {
        *pp_amf_strict_array = new amf_strict_array_t();
    }
    amf_strict_array_t *p_amf_strict_array = *pp_amf_strict_array;
    uint32_t &arr_size = p_amf_strict_array->arr_len;
    fget(ifh, (char*)&arr_size, sizeof(arr_size));
    std::reverse((uint8_t *)&arr_size, (uint8_t *)&arr_size + sizeof(arr_size));
    fprintf(parse_file, "strict_array:\n");
    for (uint32_t i = 0; i < arr_size; ++i)
    {
        fprintf(parse_file, "\tvalue%u: ", i);
        amf_data_value_t *p_amf_data = new amf_data_value_t();
        read_amf_data(ifh, parse_file, &p_amf_data);
        p_amf_strict_array->amf_data_value_lst.push_back(p_amf_data);
    }
    return arr_size;
}

const uint8_t *read_end_marker(FILE *ifh, amf_data_value_t *p_amf_marker)
{
    if (NULL == p_amf_marker)
    {
        return NULL;
    }
    amf_object_end_marker_t *p_end_marker = NULL;
    switch (p_amf_marker->type)
    {
    case AMF_TYPE_OBJECT:
        if (NULL == p_amf_marker->data_value.p_object)
        {
            p_amf_marker->data_value.p_object = new amf_object_t();
        }
        p_end_marker = &p_amf_marker->data_value.p_object->object_end_marker;
        break;
    case AMF_TYPE_ECMA_ARRAY:
        if (NULL == p_amf_marker->data_value.p_emca_array)
        {
            p_amf_marker->data_value.p_emca_array = new amf_emca_array_t();
        }
        p_end_marker = &p_amf_marker->data_value.p_emca_array->object_end_marker;
        break;
    default:
        break;
    }
    if (NULL != p_end_marker)
    {
        uint24_t &end_marker = p_end_marker->end_mark;
        fget(ifh, (char *)end_marker, sizeof(end_marker));
        assert(end_marker[0] = FLV_OBJECT_END_MARKER[0]);
        assert(end_marker[1] = FLV_OBJECT_END_MARKER[0]);
        assert(end_marker[2] = FLV_OBJECT_END_MARKER[2]);
        return end_marker;
    }
    return NULL;
}

uint8_t read_amf_data(FILE *ifh, FILE *parse_file, amf_data_value_t **pp_amf_data)
{
    if (NULL == *pp_amf_data)
    {
        *pp_amf_data = new amf_data_value_t();
    }
    uint8_t type = read_byte(ifh, &(*pp_amf_data)->type);
    switch (type)
    {
    case AMF_TYPE_NUMBER:
        {
            amf_number_t dfn = read_number(ifh, &(*pp_amf_data)->data_value.number);
            fprintf(parse_file, "%0.2lf\n", dfn);
        }
        break;
    case AMF_TYPE_BOOLEAN:
        {
            uint8_t cn = read_byte(ifh, &(*pp_amf_data)->data_value.boolean_vaule);
            fprintf(parse_file, "%d\n", cn);
        }
        break;
    case AMF_TYPE_STRING:
        {
            const uint8_t *value= read_string(ifh, &(*pp_amf_data)->data_value.string_value);
            fprintf(parse_file, "%s\n", value);
        }
        break;
    case AMF_TYPE_OBJECT:
        {
            fprintf(parse_file, "\n");
            uint32_t nobject = read_object(ifh, parse_file, &(*pp_amf_data)->data_value.p_object);
            fprintf(parse_file, "object's num = %d\n", nobject);
        }
        break;
    case AMF_TYPE_REFERENCE:
        {
            unsigned short reference = 0;
            fget(ifh, (char *)&reference, sizeof(reference));
            fprintf(parse_file, "%u\n", reference);
        }
        break;
    case AMF_TYPE_ECMA_ARRAY:
        {
            fprintf(parse_file, "\n");
            uint32_t narr = read_emca_array(ifh, parse_file, &(*pp_amf_data)->data_value.p_emca_array);
            fprintf(parse_file, "emca_array's num = %d\n", narr);
        }
        break;
    case AMF_TYPE_OBJECT_END:
        {
            read_end_marker(ifh, *pp_amf_data);
        }
        break;
    case AMF_TYPE_STRICT_ARRAY:
        {
            fprintf(parse_file, "\n");
            uint32_t nsarr = read_strict_array(ifh, parse_file, &(*pp_amf_data)->data_value.p_strict_array);
            fprintf(parse_file, "strict_array's num = %d\n", nsarr);
        }
        break;
    case AMF_TYPE_LONG_STRING:
        {
            const uint8_t *value= read_long_string(ifh, &(*pp_amf_data)->data_value.long_string_value);
            fprintf(parse_file, "%s\n", value);
        }
        break;
    default:
        break;
    }
    return type;
}

void free_amf_obj_property(amf_object_property_t *p_obj_property)
{
    if (NULL == p_obj_property)
    {
        return;
    }
    if (NULL != p_obj_property->property_name.data)
    {
        delete[] p_obj_property->property_name.data;
        p_obj_property->property_name.data = NULL;
    }
    if (NULL != p_obj_property->p_data_value)
    {
        free_amf_data(p_obj_property->p_data_value);
    }
}

void free_amf_data(amf_data_value_t *p_data_value)
{
    if (NULL == p_data_value)
    {
        return;
    }
    switch (p_data_value->type)
    {
    case AMF_TYPE_STRING:
        if (NULL != p_data_value->data_value.string_value.data)
        {
            delete[] p_data_value->data_value.string_value.data;
            p_data_value->data_value.string_value.data = NULL;
        }
        break;
    case AMF_TYPE_LONG_STRING:
        if (NULL != p_data_value->data_value.long_string_value.data)
        {
            delete[] p_data_value->data_value.long_string_value.data;
            p_data_value->data_value.long_string_value.data = NULL;
        }
        break;
    case AMF_TYPE_ECMA_ARRAY:
        if (NULL != p_data_value->data_value.p_emca_array)
        {
            amf_obj_property_list_t &object_property_lst = p_data_value->data_value.p_emca_array->object_property_lst;
            std::for_each(object_property_lst.begin(), object_property_lst.end(), &free_amf_obj_property);
            delete p_data_value->data_value.p_emca_array; p_data_value->data_value.p_emca_array = NULL;
        }
        break;
    case AMF_TYPE_STRICT_ARRAY:
        if (NULL != p_data_value->data_value.p_strict_array)
        {
            amf_script_data_list_t &amf_data_lst = p_data_value->data_value.p_strict_array->amf_data_value_lst;
            std::for_each(amf_data_lst.begin(), amf_data_lst.end(), &free_amf_data);
            delete p_data_value->data_value.p_strict_array; p_data_value->data_value.p_strict_array = NULL;
        }
        break;
    case AMF_TYPE_OBJECT:
        break;
    default:
        break;
    }
}

void dump_flv_file()
{
    FILE *xml_file = NULL;
    if ((xml_file = open_output_file(DUMP_TYPE_XML)) == NULL)
    {
        return;
    }
    fprintf(xml_file, "<?xml version='1.0' encoding='UTF-8'?>\n");
    fprintf(xml_file, "<fileset>\n");
    fprintf(xml_file, "<flv name=\"%s\">\n", g_project_name);

    const flv_hdr_t &flv_hdr = g_flv_file.flv_hdr;
    fprintf(xml_file, "<header len=\"%lu\">\n", sizeof(flv_hdr_t));
    fprintf(xml_file, "<signature>%c%c%c</signature>\n", flv_hdr.signature[0], flv_hdr.signature[1], flv_hdr.signature[2]);
    fprintf(xml_file, "<version>0x%X</version>\n", flv_hdr.version);
    fprintf(xml_file, "<flags has_audio=\"%d\" has_video=\"%d\">0x%X</flags>\n",
        (flv_hdr.flags & 0x04) != 0, (flv_hdr.flags & 0x01) != 0, flv_hdr.flags);
    fprintf(xml_file, "<data_offset>%u</data_offset>\n", flv_hdr.data_offset);
    fprintf(xml_file, "</header>\n");

    fprintf(xml_file, "<tags len=\"%lu\">\n", g_flv_file.flv_body_lst.size());

    for (std::list<flv_body_t>::const_iterator citer = g_flv_file.flv_body_lst.begin();
        citer != g_flv_file.flv_body_lst.end(); ++citer)
    {
        uint32_t datasize = 0, timestamp = 0;
        memcpy(&datasize, citer->flv_tag.data_size, sizeof(citer->flv_tag.data_size));
        memcpy(&timestamp, citer->flv_tag.timestamp, sizeof(citer->flv_tag.timestamp));
        fprintf(xml_file, "<pre_tag_size>%d</pre_tag_size>\n", citer->pre_tag_size);
        fprintf(xml_file, "<tag type=\"%s\">\n", (citer->flv_tag.tag_type == TAG_TYPE_AUDIO) ? 
            "audio" : ((citer->flv_tag.tag_type == TAG_TYPE_VIDEO) ? "video" : "script_data"));
        fprintf(xml_file, "<head len=\"%lu\">\n", sizeof(flv_tag_t));
        fprintf(xml_file, "<tagType>%d</tagType>\n", citer->flv_tag.tag_type);
        fprintf(xml_file, "<datasize>%d</datasize>\n", datasize);
        fprintf(xml_file, "<timestamp>%d</timestamp>\n", timestamp);
        fprintf(xml_file, "<timestampex>%d</timestampex>", citer->flv_tag.timestampex);
        fprintf(xml_file, "</head>\n");

        fprintf(xml_file, "<body>\n");
        switch (citer->flv_tag.tag_type)
        {
        case TAG_TYPE_AUDIO:
            {
                fprintf(xml_file, "<audio_header>\n");
                uint16_t sound_format = (citer->flv_body_data.audio_video_hdr >> 4) & 0x0F;
                uint16_t sample_rate = (citer->flv_body_data.audio_video_hdr >> 2) & 0x03;
                uint16_t sample_size = (citer->flv_body_data.audio_video_hdr >> 1) & 0x01;
                uint16_t sound_type = (citer->flv_body_data.audio_video_hdr >> 0) & 0x01;
                fprintf(xml_file, "<sound_format value=\"%2d\">%s</sound_format>\n", sound_format, audio_format_info[sound_format]);
                fprintf(xml_file, "<sound_rate value=\"%2d\">%s</sound_rate>\n", sample_rate, audio_rate_info[sample_rate]);
                fprintf(xml_file, "<sample_size value=\"%2d\">%s</sample_size>\n", sample_size, audio_sample_size_info[sample_size]);
                fprintf(xml_file, "<sound_type value=\"%2d\">%s</sound_type>\n", sound_type, audio_mono_streno_info[sound_type]);
                fprintf(xml_file, "<datasize>%d</datasize>\n", datasize);
                fprintf(xml_file, "</audio_header>\n");
            }
            break;
        case TAG_TYPE_VIDEO:
            {
                fprintf(xml_file, "<video_header>\n");
                uint16_t frame_type = (citer->flv_body_data.audio_video_hdr >> 4) & 0x0F;
                uint16_t codec_id = (citer->flv_body_data.audio_video_hdr >> 0) & 0x0F;
                fprintf(xml_file, "<frame_type value=\"%3d\">%s</frame_type>\n",
                    frame_type, video_frame_type[frame_type - 1]);
                fprintf(xml_file, "<codec_id value=\"%3d\">%s</codec_id>\n", codec_id, video_codec_info[codec_id - 1]);
                fprintf(xml_file, "<datasize>%d</datasize>\n", datasize);
                fprintf(xml_file, "</video_header>\n");
            }
            break;
        case TAG_TYPE_META:
            {
                //assert(citer->flv_body_data.amf_script_data_lst.size() == 2);
                for (amf_script_data_list_t::const_iterator ci = citer->flv_body_data.amf_script_data_lst.begin();
                    ci != citer->flv_body_data.amf_script_data_lst.end(); )
                {
                    fprintf(xml_file, "<name value=\"%s\"/>\n", (*ci)->data_value.string_value.data);
                    fprintf(xml_file, "<value>\n");
                    std::advance(ci, 1);
                    dump_meta_data(*ci, xml_file);
                    fprintf(xml_file, "</value>\n");
                    //std::for_each(amf_list.begin(), amf_list.end(), std::bind2nd(std::ptr_fun(dump_meta_data), xml_file));
                    std::advance(ci, 1);
                }
            }
            break;
        default:
            break;
        }

        fprintf(xml_file, "</body>\n");
        fprintf(xml_file, "</tag>\n");
    }

    fprintf(xml_file, "</tags>\n");

    fprintf(xml_file, "</flv>\n");
    fprintf(xml_file, "</fileset>\n");
}

void dump_meta_data(amf_data_value_t *p_data_value, FILE *xml_file)
{
    if (NULL == p_data_value)
    {
        return;
    }
    //fprintf(xml_file, "<type>%d</type>\n", p_data_value->type);
    switch (p_data_value->type)
    {
    case AMF_TYPE_NUMBER:
        {
            fprintf(xml_file, "<number value=\"%.2f\"/>\n", p_data_value->data_value.number);
        }
        break;
    case AMF_TYPE_BOOLEAN:
        {
            fprintf(xml_file, "<boolean value=\"%d\"/>\n", p_data_value->data_value.boolean_vaule);
        }
        break;
    case AMF_TYPE_STRING:
        {
            fprintf(xml_file, "<string value=\"%s\"/>\n", p_data_value->data_value.string_value.data);
        }
        break;
    case AMF_TYPE_OBJECT:
        {
            amf_object_t *p_amf_obj = p_data_value->data_value.p_object;
            if (NULL == p_amf_obj)
            {
                break;
            }
            fprintf(xml_file, "<object>\n");
            for (amf_obj_property_list_t::const_iterator citer = p_amf_obj->object_property_lst.begin();
                citer != p_amf_obj->object_property_lst.end(); ++citer)
            {
                fprintf(xml_file, "<name value=\"%s\">\n", (*citer)->property_name.data);
                dump_meta_data((*citer)->p_data_value, xml_file);
                fprintf(xml_file, "</name>\n");
            }
            fprintf(xml_file, "</object>\n");
        }
        break;
    case AMF_TYPE_REFERENCE:
        {
        }
        break;
    case AMF_TYPE_ECMA_ARRAY:
        {
            amf_emca_array_t *p_emca_array = p_data_value->data_value.p_emca_array;
            if (NULL == p_emca_array)
            {
                break;
            }
            fprintf(xml_file, "<ecma_array>\n");
            for (amf_obj_property_list_t::const_iterator citer = p_emca_array->object_property_lst.begin();
                citer != p_emca_array->object_property_lst.end(); ++citer)
            {
                fprintf(xml_file, "<name value=\"%s\">\n", (*citer)->property_name.data);
                dump_meta_data((*citer)->p_data_value, xml_file);
                fprintf(xml_file, "</name>\n");
            }
            fprintf(xml_file, "</ecma_array>\n");
        }
        break;
    case AMF_TYPE_OBJECT_END:
        {
            fprintf(xml_file, "<obj_end>%d</obj_end>\n", AMF_TYPE_OBJECT_END);
        }
        break;
    case AMF_TYPE_STRICT_ARRAY:
        {
            amf_strict_array_t *p_strict_arr = p_data_value->data_value.p_strict_array;
            if (NULL == p_strict_arr)
            {
                break;
            }
            fprintf(xml_file, "<strict_array>\n");
            std::for_each(p_strict_arr->amf_data_value_lst.begin(), p_strict_arr->amf_data_value_lst.end(),
                std::bind2nd(std::ptr_fun(dump_meta_data), xml_file));
            fprintf(xml_file, "</strict_array>\n");
        }
        break;
    case AMF_TYPE_LONG_STRING:
        {
            fprintf(xml_file, "<str>%s</str>\n", p_data_value->data_value.long_string_value.data);
        }
        break;
    default:
        break;
    }
}

//fget - fill a buffer or structure with bytes from a file   
uint32_t fget(FILE *fh, char *p, uint32_t s) {
    uint32_t i = 0;
    for (; i<s; i++)   
        *(p+i) = (char)fgetc(fh);   
    return i;   
}   

//fput - write a buffer or structure to file   
uint32_t fput(FILE *fh, char *p, uint32_t s) {
    uint32_t i = 0;   
    for (; i<s; i++)   
        fputc(*(p+i), fh);   
    return i;   
}   

//utility function to overwrite memory   
uint32_t copymem(char *d, char *s, uint32_t c) {
    uint32_t i = 0;
    for (; i<c; i++)   
        *(d+i) = *(s+i);   
    return i;   
}

//xfer - transfers *count* bytes from an input file to an output file   
uint32_t xfer(FILE *ifh, FILE *ofh, uint32_t c) {
    uint32_t i = 0;
    for (; i<c; i++)   
        fputc(fgetc(ifh),ofh);   
    return i;   
}   

//This function handles iterative file naming and opening   
FILE* open_output_file(uint8_t tag) {   

    //instantiate two buffers   
    char file_name[_MAX_FNAME] = { 0 }, ext[_MAX_EXT] = { 0 };
    switch (tag)
    {
    case DUMP_TYPE_DEFAULT:
        //determine the file extension   
        strcpy(ext, "txt\0");
        break;
    case DUMP_TYPE_XML:
        //determine the file extension   
        strcpy(ext, "xml\0");
        break;
    case TAG_TYPE_AUDIO:
        //determine the file extension   
        strcpy(ext, "mp3\0");
        break;
    case TAG_TYPE_VIDEO:
        //determine the file extension   
        strcpy(ext, "flv\0");
        break;
    default:
        //determine the file extension   
        strcpy(ext, "txt\0");
        break;
    }

    //build the file name   
    sprintf(file_name, "%s_%i.%s", g_project_name, g_cur_num, ext);   

    //return the file pointer   
    return fopen(file_name, "wb");   
}   

//read in the cue points from file in a list format   
uint32_t * read_cue_file(char *fn) {   
    FILE * cfh;   
    uint32_t ms, n, count = 0;   
    char sLine[13];   
    unsigned int ts[5];   
    float ts_f = 0;   

    //instantiate the heap pointer   
    uint32_t * p = (uint32_t *) malloc((uint32_t) 4);    

    //try opening the cue file   
    if ( (cfh = fopen(fn, "r")) != NULL) {   

        //grab the first string   
        n = fscanf(cfh, "%12s", sLine);   

        //loop until there are no more strings   
        while (n==1) {     

            //reset milliseconds   
            ms = 0;   

            //check to see if in timestamp format   
            if ((sLine[2]==':') && (sLine[5]==':') && (sLine[8]==':')) {   

                //replace the colons with tabs   
                sLine[2]='\t'; sLine[5]='\t'; sLine[8]='\t';   

                //extract the timestamp values
				sscanf(sLine, "%u %u %u %u", &ts[0], &ts[1], &ts[2], &ts[3]);

                //calculate the timestamp as milliseconds   
                ms = (ts[0]*3600 + ts[1]*60 + ts[2])*1000 + ts[3];   

            } else {   

                //just see if there is a decimal notation of milliseconds   
                sscanf(sLine, "%f", &ts_f);   
                ms = (uint32_t) ts_f * 1000;   

            }   

            //if a cuepoint was found on this line   
            if (ms > 0) {   

                //dynamically reallocate memory space as necessary   
                if (count % CUE_BLOCK_SIZE == 0)   
                    p = (uint32_t *) realloc(p, (uint32_t) ((count + CUE_BLOCK_SIZE)*sizeof(int)));   

                //set the cue value   
                *(p + count++) = ms;   

            }   

            //grab the next string   
            n = fscanf(cfh, "%12s", sLine);   
        }   
    }   

    //set the last cue point to max int   
    *(p+count) = 0xFFFFFFFF;   

    //return the pointer to the heap allocation   
    return p;   
}
