
% Importing all the data from the txt file
originalHist = importdata("originalHist.txt");
compressedHist = importdata("compressedHist.txt");

hist_y_original = originalHist(:,1); 
hist_u_original = originalHist(:,2); 
hist_v_original = originalHist(:,3); 

% eleminar o shift feito
hist_y_compressed = compressedHist(:,1); 
hist_u_compressed = compressedHist(:,2); 
hist_v_compressed = compressedHist(:,3); 

pixelValues = 0:255;
figure(1)
sgtitle('Histograma do vídeo original')
subplot(3,1,1)
bar(pixelValues,hist_y_original)
xlim([0 255])
title('Componente Y')
ylabel("Frequência")

subplot(3,1,2)
bar(pixelValues,hist_u_original)
xlim([0 255])
title('Componente U')
ylabel("Frequência")

subplot(3,1,3)
bar(pixelValues,hist_v_original)
xlim([0 255])
title('Componente V')
xlabel("Valor do pixel")
ylabel("Frequência")

f = gcf;
exportgraphics(f,'originalVideo.png','Resolution',300)

figure(2)
sgtitle('Histograma do vídeo comprimido')
subplot(3,1,1)
bar([-128:128], hist_y_compressed(255-128:255+128))

title('Componente Y')
ylabel("Frequência")

subplot(3,1,2)
bar([-128:128], hist_u_compressed(255-128:255+128))

title('Componente U')
ylabel("Frequência")

subplot(3,1,3)
bar([-128:128], hist_v_compressed(255-128:255+128))

title('Componente V')
xlabel('Valor do pixel')
ylabel("Frequência")

f = gcf;
exportgraphics(f,'compressedVideo.png','Resolution',300)
