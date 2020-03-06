#ifndef __CCP
#define __CCP

#include <vector>
#include <utility>
#include <map>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include "random.h"

class CCP {

private:
   int n_cluster;
   std::vector<std::vector<double>> posiciones;
   std::vector<std::vector<double>> centroides;
   std::map<std::pair<int,int>,int> restricciones;
   std::vector<std::vector<int>> clusters;
   std::vector<double> d_intracluster;
   std::vector<int> solucion;
   double desv_gen;
   int infactibilidad;

   void cargar_posiciones(const std::string archivo);
   void cargar_restricciones(const std::string archivo);
   void calcular_centroide(const int i);
   void distancia_intracluster(const int i);
   void desviacion_general();
   void generar_vecino();
   void leer_solucion();
   void generar_solucion();
   void asignar_cluster(const int n);
   double distancia_nodo_cluster(const int n, const int c);
   int restricciones_incumplidas(const int n, const int c);
   void limpiar_clusters();
   //void infactibilidad_total();
   bool solucion_factible();

public:
   CCP(const int n, const std::string p, const std::string r);
   void mostrar_datos();
   void mostrar_solucion(int i);
   int greedy();
   void busqueda_local();
};


#endif
