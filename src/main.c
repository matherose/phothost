#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>


int transcode_image(const char* in_filename, const char* out_filename) {
    int ret;
    int video_stream_index = -1;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVCodecContext *dec_ctx = NULL, *enc_ctx = NULL;
    AVPacket packet, out_packet;
    AVFrame *frame = NULL;
    AVStream *out_stream = NULL;

    // Initialisation de l'entrée
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, NULL, NULL)) < 0) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir %s\n", in_filename);
        return ret;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        fprintf(stderr, "Erreur: Infos du stream non trouvées\n");
        goto end;
    }
    // Récupérer le stream vidéo
    for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
        if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index < 0) {
        fprintf(stderr, "Erreur: Aucun stream vidéo trouvé\n");
        ret = -1;
        goto end;
    }
    // Initialisation du décodeur
    const AVCodec *dec = avcodec_find_decoder(ifmt_ctx->streams[video_stream_index]->codecpar->codec_id);
    if (!dec) {
        fprintf(stderr, "Erreur: Décodeur non trouvé\n");
        ret = -1;
        goto end;
    }
    dec_ctx = avcodec_alloc_context3(dec);
    avcodec_parameters_to_context(dec_ctx, ifmt_ctx->streams[video_stream_index]->codecpar);
    if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le décodeur\n");
        goto end;
    }
    // Allocation de la frame
    frame = av_frame_alloc();
    if (!frame) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    av_init_packet(&packet);
    // Lire la première frame (image unique)
    if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0) {
        fprintf(stderr, "Erreur: Impossible de lire la frame\n");
        goto end;
    }
    if(packet.stream_index != video_stream_index) {
        fprintf(stderr, "Erreur: Packet non vidéo reçu\n");
        ret = -1;
        goto end;
    }
    ret = avcodec_send_packet(dec_ctx, &packet);
    if(ret < 0) {
        fprintf(stderr, "Erreur: Envoi du packet au décodeur\n");
        goto end;
    }
    ret = avcodec_receive_frame(dec_ctx, frame);
    if(ret < 0) {
        fprintf(stderr, "Erreur: Réception de la frame décodée\n");
        goto end;
    }
    av_packet_unref(&packet);
    
    // Préparation du contexte de sortie
    if ((ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename)) < 0 || !ofmt_ctx) {
        fprintf(stderr, "Erreur: Création du contexte de sortie\n");
        goto end;
    }
    // Trouver l'encodeur AV1 (adapté pour AVIF)
    AVCodec *enc = avcodec_find_encoder(AV_CODEC_ID_AV1);
    if (!enc) {
        fprintf(stderr, "Erreur: Encodeur AV1 non trouvé\n");
        ret = -1;
        goto end;
    }
    out_stream = avformat_new_stream(ofmt_ctx, NULL);
    if (!out_stream) {
        fprintf(stderr, "Erreur: Allocation du stream de sortie\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    enc_ctx = avcodec_alloc_context3(enc);
    if (!enc_ctx) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    // Configuration de l'encodeur (les paramètres peuvent être ajustés)
    enc_ctx->height = frame->height;
    enc_ctx->width = frame->width;
    enc_ctx->sample_aspect_ratio = frame->sample_aspect_ratio;
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    enc_ctx->time_base = (AVRational){1, 25};
    if ((ret = avcodec_open2(enc_ctx, enc, NULL)) < 0) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir l'encodeur\n");
        goto end;
    }
    ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
    if(ret < 0) {
        fprintf(stderr, "Erreur: Copie des paramètres de l'encodeur\n");
        goto end;
    }
    // Ouverture du fichier de sortie
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        if ((ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE)) < 0) {
            fprintf(stderr, "Erreur: Ouverture du fichier de sortie %s\n", out_filename);
            goto end;
        }
    }
    // Ecriture de l'en-tête
    if ((ret = avformat_write_header(ofmt_ctx, NULL)) < 0) {
        fprintf(stderr, "Erreur: Ecriture de l'en-tête de sortie\n");
        goto end;
    }
    // Encodage de la frame
    ret = avcodec_send_frame(enc_ctx, frame);
    if(ret < 0) {
        fprintf(stderr, "Erreur: Envoi de la frame à l'encodeur\n");
        goto end;
    }
    av_init_packet(&out_packet);
    ret = avcodec_receive_packet(enc_ctx, &out_packet);
    if(ret < 0) {
        fprintf(stderr, "Erreur: Réception du packet encodé\n");
        goto end;
    }
    out_packet.stream_index = out_stream->index;
    if ((ret = av_interleaved_write_frame(ofmt_ctx, &out_packet)) < 0) {
        fprintf(stderr, "Erreur: Ecriture du packet de sortie\n");
        goto end;
    }
    av_packet_unref(&out_packet);
    av_write_trailer(ofmt_ctx);

    printf("Transcodage réel de %s vers %s réussi.\n", in_filename, out_filename);
    ret = 0;
end:
    // Libérer les ressources
    if (dec_ctx) avcodec_free_context(&dec_ctx);
    if (enc_ctx) avcodec_free_context(&enc_ctx);
    if (ifmt_ctx) avformat_close_input(&ifmt_ctx);
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    if (ofmt_ctx) avformat_free_context(ofmt_ctx);
    if (frame) av_frame_free(&frame);
    return ret;
}

int main(void) {
    printf("Hello, World!\n");
    printf("FFmpeg version: %s\n", av_version_info());
    printf("libavcodec version: %u\n", avcodec_version());
    printf("libavformat version: %u\n", avformat_version());
    
    // Vérification du fonctionnement de FFmpeg
    printf("Les bibliothèques FFmpeg fonctionnent correctement.\n");

    // Exécution du transcoding réel
    printf("Transcoder src/image.jpg vers src/image.avif...\n");
    if(transcode_image("src/image.jpg", "src/image.avif") == 0) {
        printf("Transcodage réel réussi.\n");
    } else {
        printf("Échec du transcodage réel.\n");
    }

    return 0;
}
