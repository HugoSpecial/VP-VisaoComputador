#include <iostream>
#include <string>
#include <chrono>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>

extern "C" {
#include "vc.h"
}


void vc_timer(void) {
	static bool running = false;
	static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

	if (!running) {
		running = true;
	}
	else {
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

		// Tempo em segundos.
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
		double nseconds = time_span.count();

		std::cout << "Tempo decorrido: " << nseconds << "segundos" << std::endl;
		std::cout << "Pressione qualquer tecla para continuar...\n";
		std::cin.get();
	}
}


int main(void) {
	// Vídeo
	char videofile[20] = "video1.mp4";
	cv::VideoCapture capture;
	struct
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;
	// Outros
	std::string str;
	int key = 0;

	/* Leitura de vídeo de um ficheiro */
	/* NOTA IMPORTANTE:
	O ficheiro video.avi deverá estar localizado no mesmo directório que o ficheiro de código fonte.
	*/
	capture.open(videofile);

	/* Em alternativa, abrir captura de vídeo pela Webcam #0 */
	//capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

	/* Verifica se foi possível abrir o ficheiro de vídeo */
	if (!capture.isOpened())
	{
		std::cerr << "Erro ao abrir o ficheiro de vídeo!\n";
		return 1;
	}

	/* Número total de frames no vídeo */
	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	/* Frame rate do vídeo */
	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
	/* Resolução do vídeo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o vídeo */
	cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);

	/* Inicia o timer */
	vc_timer();

	cv::Mat frame;
	while (key != 'q') {
		/* Leitura de uma frame do vídeo */
		capture.read(frame);

		/* Verifica se conseguiu ler a frame */
		if (frame.empty()) break;

		/* Número da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

<<<<<<< HEAD

		// Faça o seu código aqui...

		// Criação das imagens para processamento
		IVC* imageFromVideo = vc_image_new(video.width, video.height, 3, 255);
		IVC* imageInRGB = vc_image_new(video.width, video.height, 3, 255);
		IVC* imageInHSV = vc_image_new(video.width, video.height, 3, 255);
		IVC* imageSegmentation = vc_image_new(video.width, video.height, 1, 255);

		// Copiar dados da frame para o buffer de processamento
		memcpy(imageFromVideo->data, frame.data, video.width * video.height * 3);



		if (vc_bgr_to_rgb(imageFromVideo, imageInRGB) == 0) {
			std::cerr << "Erro na conversão de BGR para RBG!\n";
			vc_image_free(imageFromVideo);
			vc_image_free(imageInRGB);
			vc_image_free(imageInHSV);
			vc_image_free(imageSegmentation);
			return 1;
		}


		cv::Mat rgbImage(video.height, video.width, CV_8UC3, imageInRGB->data);
		cv::cvtColor(rgbImage, rgbImage, cv::COLOR_RGB2BGR);
		cv::imshow("BGR to RGB", rgbImage);

		// Conversão para HSV
		if (vc_rgb_to_hsv(imageInRGB, imageInHSV) == 0) {
			std::cerr << "Erro na conversão de RGB para HSV!\n";
			vc_image_free(imageFromVideo);
			vc_image_free(imageInRGB);
			vc_image_free(imageInHSV);
			vc_image_free(imageSegmentation);
			return 1;
		}
		cv::imshow("BGR to RGB", rgbImage);

		// Segmentação HSV

		if (vc_hsv_segmentation(imageInHSV, imageSegmentation, 210, 230, 30, 100, 30, 60) != 1) {
			std::cerr << "Erro na segmentação HSV!\n";
			vc_image_free(imageFromVideo);
			vc_image_free(imageInHSV);
			vc_image_free(imageSegmentation);
			return 1;
		}

		// Exibição da imagem segmentada frame a frame
		cv::Mat segImage(video.height, video.width, CV_8UC1, imageSegmentation->data);
		//cv::imshow("Segmentacao", segImage);
		cv::imshow("VC - VIDEO", frame);






		/* douradas */
		// Cria novas imagens IVC  
		IVC* image3 = vc_image_new(video.width, video.height, 3, 255);
		IVC* image4 = vc_image_new(video.width, video.height, 3, 255);
		IVC* image5 = vc_image_new(video.width, video.height, 3, 255);

		vc_bgr_to_rgb(imageInRGB, image3);
		vc_rgb_to_hsv(image3, image4);
		vc_hsv_segmentation(image4, image5, 40, 70, 20, 70, 20, 70);

		// Mostra a imagem hsv 
		memcpy(frame.data, imageInHSV->data, video.width * video.height * 3);
		cv::imshow("VC - VIDEO", frame);

		// Cria um Mat com os dados da imagem segmentada e mostra
		cv::Mat segmentedImage(video.height, video.width, CV_8UC1, image5->data);
		cv::imshow("Segmentacao HSV", segmentedImage);

		// Liberta a memória da imagem IVC que havia sido criada  
		vc_image_free(image3);
		vc_image_free(image4);
		vc_image_free(image5);

		/* escuras */
		// Cria novas imagens IVC  
		IVC* image6 = vc_image_new(video.width, video.height, 3, 255);
		IVC* image7 = vc_image_new(video.width, video.height, 3, 255);
		IVC* image8 = vc_image_new(video.width, video.height, 3, 255);

		vc_bgr_to_rgb(imageInRGB, image6);
		vc_rgb_to_hsv(image6, image7);
		vc_hsv_segmentation(image7, image8, 30, 45, 50, 75, 20, 35);

		// Mostra a imagem hsv 
		memcpy(frame.data, imageInHSV->data, video.width * video.height * 3);
		cv::imshow("VC - VIDEO", frame);

		// Cria um Mat com os dados da imagem segmentada e mostra
		cv::Mat segmentedImage2(video.height, video.width, CV_8UC1, image8->data);
		cv::imshow("Segmentacao HSV", segmentedImage2);

		// Liberta a memória da imagem IVC que havia sido criada  
		vc_image_free(image6);
		vc_image_free(image7);
		vc_image_free(image8);





		// Aguardar a tecla 'q' para sair
		key = cv::waitKey(60);

		// Liberação dos recursos
		vc_image_free(imageFromVideo);
		vc_image_free(imageInHSV);
		vc_image_free(imageSegmentation);


=======
		// Faça o seu código aqui...
		/*
		// Cria uma nova imagem IVC
		IVC *image = vc_image_new(video.width, video.height, 3, 255);
		// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
		memcpy(image->data, frame.data, video.width * video.height * 3);
		// Executa uma função da nossa biblioteca vc
		vc_rgb_get_green(image);
		// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
		memcpy(frame.data, image->data, video.width * video.height * 3);
		// Liberta a memória da imagem IVC que havia sido criada
		vc_image_free(image);
		*/
>>>>>>> parent of 646d8aa (Convert BGR to RGB and Inicial Segmentation)
		// +++++++++++++++++++++++++

		/* Exemplo de inserção texto na frame */
		str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

		/* Exibe a frame */
		cv::imshow("VC - VIDEO", frame);

		/* Sai da aplicação, se o utilizador premir a tecla 'q' */
		key = cv::waitKey(1);
	}

	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de vídeo */
	capture.release();

	return 0;
}
