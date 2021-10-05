# Dockerfile designed around using "score_tracks" utility from KWANT:
# OS -> Ubuntu:18.04
# ENTRYPOINT: "score_tracks"

FROM ubuntu:18.04

RUN apt-get update \
&&  apt-get install -y git \
			build-essential \
			cmake

RUN git clone https://github.com/Kitware/fletch.git fletch-source \
 && git clone https://github.com/Kitware/kwiver.git kwiver-source \
 && git clone https://github.com/Kitware/kwant.git kwant-source \
 && mkdir fletch-build \
 && cd fletch-build \
 && cmake ../fletch-source -Dfletch_ENABLE_Boost:BOOL=ON \
	-Dfletch_ENABLE_Eigen:BOOL=ON \
	-Dfletch_ENABLE_GeographicLib:BOOL=ON \
	-Dfletch_ENABLE_TinyXML1:BOOL=ON \
	-Dfletch_ENABLE_TinyXML2:BOOL=ON \
	-Dfletch_ENABLE_VXL:BOOL=ON \
	-Dfletch_ENABLE_YAMLcpp:BOOL=ON \
	-Dfletch_ENABLE_ZLib:BOOL=ON \
	-Dfletch_ENABLE_libjson:BOOL=ON \
	-Dfletch_ENABLE_shapelib:BOOL=ON \
	-Dfletch_ENABLE_libgeotiff:BOOL=ON \
	-Dfletch_ENABLE_libtiff:BOOL=ON \
 && make -j16 \
 && mkdir /kwiver-build \
 && cd /kwiver-build \
 && cmake ../kwiver-source -Dfletch_DIR:PATH=/fletch-build \
	-DKWIVER_ENABLE_ARROWS:BOOL=ON \
	-DKWIVER_ENABLE_KPF:BOOL=ON \
	-DKWIVER_ENABLE_TRACK_ORACLE:BOOL=ON \
	-DKWIVER_ENABLE_VXL:BOOL=ON \
 && make -j16 \
 && mkdir /kwant-build \
 && cd /kwant-build \
 && cmake ../kwant-source -Dkwiver_DIR=/kwiver-build/ \
 && make -j16 \
 && rm -rf fletch-source \
 && rm -rf kwiver-source \
 && rm -rf kwant-source

ENV VG_PLUGIN_PATH=/kwiver-build
ENV PATH=/kwiver-build/bin:$PATH
ENV LD_LIBRARY_PATH="/kwiver-build/lib:$LD_LIBRARY_PATH"
ENV KWIVER_PLUGIN_PATH=/kwiver-build/lib/kwiver/modules:/kwiver-build/lib/kwiver/processes:$KWIVER_PLUGIN_PATH
ENV KWIVER_CONFIG_PATH=/kwiver-build/share/kwiver/1.4.0/config
ENV LD_LIBRARY_PATH=/fletch-build/install/lib:$LD_LIBRARY_PATH
ENV GDAL_DATA=/share/gdal
ENV PROJ_LIB=/share/proj
ENV KWIVER_DEFAULT_LOG_LEVEL=WARN

WORKDIR /kwant-build/bin

ENTRYPOINT ["./score_tracks"]

CMD ["-Help"]
