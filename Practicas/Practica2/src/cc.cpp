#include "cc.h"

/////////////////////////////////////////////////////////////////////////////////
CCP::CCP(const int n, const std::string p, const std::string r){
   n_cluster = n;
   desv_gen = 0;
   infactibilidad = 0;
   lambda = 0;
   f_objetivo = 0;
   poblacion = 0;
   ind_eval = 0;
   cargar_posiciones(p);
   cargar_restricciones(r);
   centroides.resize(n_cluster);
   clusters.resize(n_cluster);

   double max = posiciones[0][0], min = posiciones[0][0];
   for( unsigned i = 0; i < posiciones.size(); i++){
      for( unsigned j = 0; j < posiciones[i].size(); j++){
         if(posiciones[i][j] < min){
            min = posiciones[i][j];
         }
         if (posiciones[i][j] > max){
            max = posiciones[i][j];
         }
      }
   }
   for( int i = 0; i < n_cluster; i++){
      for( unsigned j = 0; j < posiciones[0].size(); j++){
         centroides[i].push_back(Randfloat(min,max-1));
      }
   }
   d_intracluster.resize(n_cluster);
   solucion.resize(posiciones.size());
   for( unsigned i = 0; i < solucion.size(); i++){
      solucion[i] = -1;
   }

   calcular_lambda();
}
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
void CCP::cargar_posiciones(const std::string archivo){
   std::ifstream in(archivo);
   int i = 0;
   std::string variable;
   std::string dato;
   while(getline(in,variable)){
      posiciones.resize(i+1);
      std::istringstream iss(variable);
      while(getline(iss,dato,',')){
         posiciones[i].push_back(atof(dato.c_str()));
      }
      i++;
   }
}

void CCP::cargar_restricciones(const std::string archivo){
   std::ifstream in(archivo);
   int valor;
   int i = 0,j = 0;
   std::string fila;
   std::string restriccion;
   while(getline(in,fila)){
      std::istringstream iss(fila);
      while(getline(iss,restriccion,',')){
         std::pair<int,int> pareja(i,j);
         valor = atof(restriccion.c_str());
         if (valor != 0 && i != j){
            restricciones.insert(std::pair<std::pair<int,int>,int>(pareja,valor));
         }
         j++;
      }
      i++;
      j=0;
   }

}
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
void CCP::calcular_centroide(const int i){
   for( unsigned j = 0; j < centroides[i].size(); j++){
      centroides[i][j] = 0;
   }
   for( unsigned j = 0; j < clusters[i].size(); j++){
      for( unsigned k = 0; k < posiciones[clusters[i][j]].size(); k++){
            centroides[i][k] += (1.0/clusters[i].size()) * posiciones[clusters[i][j]][k];
      }
   }
}

void CCP::distancia_intracluster(const int i){
   double d_euclidea;
   for( int j = 0; j < n_cluster; j++){
      d_intracluster[j] = 0;
   }
   for( unsigned j = 0; j < clusters[i].size(); j++){
      for( unsigned k = 0; k < posiciones[clusters[i][j]].size(); k++){
         d_euclidea =  std::abs(posiciones[clusters[i][j]][k] - centroides[i][k]);
         d_euclidea *= d_euclidea;
         d_intracluster[i] += (1.0/clusters[i].size()) * d_euclidea;
      }
   }
}

void CCP::desviacion_general(){
   desv_gen = 0;
   for( int i = 0; i < n_cluster; i++){
      distancia_intracluster(i);
      desv_gen += (1.0/n_cluster)*d_intracluster[i];
   }
}

void CCP::calcular_lambda(){
   lambda = 0;
   double d, d_max = 0.0;
   for( int i = 0; i < (int) posiciones.size(); i++){
      for( int j = i+1; j < (int) posiciones.size(); j++){
         d = distancia_nodo_nodo(i,j);
         if(d > d_max){
            d_max = d;
         }
      }
   }
   lambda = d_max / restricciones.size();
   //std::cout << "Distancia maxima del conjunto: " << d_max << std::endl;
   //std::cout << "Numero de restricciones: " << restricciones.size() << std::endl;
   //std::cout << "Lambda: " << lambda << std::endl;
}

void CCP::infactibilidad_solucion(){
   infactibilidad = calcular_infact_sol(solucion);
}
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
double CCP::distancia_nodo_cluster(const int n, const int c){
   double d_euclidea = 0, componente = 0;
   for( unsigned i = 0; i < posiciones[n].size(); i++){
      componente = std::abs(posiciones[n][i] - centroides[c][i]);
      componente *= componente;
      d_euclidea += componente;
   }
   return d_euclidea;
}

double CCP::distancia_nodo_nodo(const int n, const int m){
   double d_euclidea = 0, componente = 0;
   for( unsigned i = 0; i < posiciones[n].size(); i++){
      componente = std::abs(posiciones[n][i] - posiciones[m][i]);
      componente *= componente;
      d_euclidea += componente;
   }
   return d_euclidea;
}

double CCP::restricciones_incumplidas(const int n, const int c){
   double incumplidas = 0;
   std::pair<int,int> pareja;
   pareja.first = n;
   for( unsigned i = 0; i < clusters[c].size(); i++){
      pareja.second = clusters[c][i];
      auto it = restricciones.find(pareja);
      if(it->second == -1 && it != restricciones.end()){
         incumplidas++;
      }
   }
   for( int i = 0; i < n_cluster; i++){
      if(i != c){
         for( unsigned j = 0; j < clusters[i].size(); j++){
            pareja.second = clusters[i][j];
            auto it = restricciones.find(pareja);
            if(it->second == 1 && it != restricciones.end()){
               incumplidas++;
            }
         }
      }
   }
   return incumplidas;
}
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
void CCP::asignar_cluster(const int n){
   std::pair<int,int> pareja;
   std::vector<std::pair<int,int>> r;

   int cluster = -1;
   double d_min, d, r_min;

   for( int i = 0; i < n_cluster; i++){
      pareja.first = restricciones_incumplidas(n,i);
      pareja.second = i;
      r.push_back(pareja);
   }

   std::sort(r.begin(),r.end());

   r_min = r[0].first;
   d_min = 1000000.0;
   for( unsigned i = 0; i < r.size() && r[i].first == r_min; i++){
      d = distancia_nodo_cluster(n,r[i].second);
      if(d < d_min){
         d_min = d;
         cluster = r[i].second;
      }
   }
   clusters[cluster].push_back(n);
}

int CCP::buscar_cluster(const int n){
   for( int i = 0; i < n_cluster; i++){
      for( unsigned j = 0; j < clusters[i].size(); j++){
         if(clusters[i][j] == n){
            return i;
         }
      }
   }
   return -1;
}

void CCP::limpiar_clusters(){
   for( int i = 0; i < n_cluster; i++){
      clusters[i].clear();
   }
}
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
std::vector<int> CCP::crear_solucion(){
   std::vector<int> sol;
   std::vector<int> index;
   std::vector<std::vector<int>> c;
   c.resize(n_cluster);
   sol.resize(posiciones.size());

   for( unsigned i = 0; i < posiciones.size(); i++){
      index.push_back(i);
   }
   std::random_shuffle(index.begin(), index.end(), Randint_shuffle);
   auto it = index.begin();
   for( int i = 0; i < n_cluster; i++, ++it){
      c[i].push_back(*it);
   }
   int random;
   for(; it != index.end(); ++it){
      random = Randint(0,n_cluster);
      c[random].push_back(*it);
   }

   for( int i = 0; i < n_cluster; i++){
      for( unsigned j = 0; j < c[i].size(); j++){
         sol[c[i][j]] = i;
      }
   }

   return sol;
}

void CCP::solucion_inicial(){
   solucion = crear_solucion();

   for(int i = 0; i < n_cluster; i++){
      calcular_centroide(i);
   }
}

void CCP::generar_solucion(){
   for( int i = 0; i < n_cluster; i++){
      for( unsigned j = 0; j < clusters[i].size(); j++){
         solucion[clusters[i][j]] = i;
      }
   }
}

void CCP::generar_vecino(){
   bool salir = false;
   int pos, n;
   while(!salir && quedan_vecinos()){
      salir = false;
      pos = Randint(0,solucion.size());
      n = Randint(0,n_cluster);
      std::pair<int,int> pareja = std::make_pair(pos,n);
      auto it = vecindario.find(pareja);
      if(it != vecindario.end()){
         salir = true;
         vecindario.erase(it);
         for( int i = 0; i < n_cluster; i++){
            clusters[i].clear();
         }
         for(unsigned j = 0; j < solucion.size(); j++){
               clusters[solucion[j]].push_back(j);
         }
         solucion[pos] = n;
         clusters[solucion[pos]].push_back(pos);
         for( int i = 0; i < n_cluster; i++){
            calcular_centroide(i);
         }
         desviacion_general();
         infactibilidad_solucion();
         f_objetivo = desv_gen + (infactibilidad*lambda);
      }
   }
}

void CCP::generar_vecindario(){
   vecindario.clear();
   for(unsigned i = 0; i < posiciones.size(); i++){
      for(int j = 0; j < n_cluster; j++){
         if(j != buscar_cluster(i) && clusters[buscar_cluster(i)].size() > (unsigned) 1){
            vecindario.insert(std::make_pair(i,j));
         }
      }
   }
}

void CCP::leer_solucion(){
   limpiar_clusters();
   for( unsigned i = 0; i < solucion.size(); i++){
      clusters[solucion[i]].push_back(i);
   }
   for( int i = 0; i < n_cluster; i++){
      calcular_centroide(i);
   }
   desviacion_general();
   infactibilidad_solucion();
   f_objetivo = desv_gen + (infactibilidad*lambda);
}

void CCP::leer_vecino(){
   limpiar_clusters();
   for( unsigned i = 0; i < solucion.size(); i++){
      clusters[solucion[i]].push_back(i);
   }
   for(int i = 0; i < n_cluster; i++){
      calcular_centroide(i);
   }
   desviacion_general();
   f_objetivo = desv_gen + (infactibilidad*lambda);
}

bool CCP::quedan_vecinos(){
   if(vecindario.size() > 0){
      return true;
   }
   return false;
}
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
void CCP::generacion_inicial(){
   for(int i = 0; i < poblacion; i++){
      generacion.push_back(crear_solucion());
   }
   for(auto it = generacion.begin(); it != generacion.end(); ++it){
      f_generacion.push_back(evaluar_solucion(*it));
   }

   seleccionar_mejor();
}

int CCP::calcular_infact_sol(std::vector<int> & sol){
   int infactibilidad = 0;
   for(int i = 0; i < (int) sol.size(); i++){
      for(int j = i+1; j < (int) sol.size(); j++){
         std::pair<int,int> pareja = std::make_pair(i,j);
         auto it = restricciones.find(pareja);
         if(it != restricciones.end()){
            if(it->second == -1 && sol[i] == sol[j]){
               infactibilidad++;
            } else if(it->second == 1 && sol[i] != sol[j]){
               infactibilidad++;
            }
         }
      }
   }
   return infactibilidad;
}

double CCP::evaluar_solucion(std::vector<int> & sol){
   ind_eval++;

   std::vector<std::vector<double>> cent;
   std::vector<double> d_intra;
   std::vector<std::vector<int>> c;
   double desv_sol;
   double d_euclidea;
   int infact;
   double evaluacion;

   c.resize(n_cluster);
   for( int i = 0; i < (int) sol.size(); i++){
      c[sol[i]].push_back(i);
   }

   cent.resize(n_cluster);
   for( int i = 0; i < n_cluster; i++){
      cent[i].resize(posiciones[0].size());
   }
   for( int i = 0; i < n_cluster; i++){
      for(int j = 0; j < (int) posiciones[0].size(); j++){
         cent[i].push_back(0);
      }
   }
   for(int i = 0; i < n_cluster; i++){
      for( int j = 0; j < (int) c[i].size(); j++){
         for( int k = 0; k < (int) posiciones[c[i][j]].size(); k++){
               cent[i][k] += (1.0/c[i].size()) * posiciones[c[i][j]][k];
         }
      }
   }

   d_intra.resize(n_cluster);
   for(int i = 0; i < n_cluster; i++){
      for( int j = 0; j < n_cluster; j++){
         d_intracluster.push_back(0);
      }
      for( int j = 0; j < (int) c[i].size(); j++){
         for( int k = 0; k < (int) posiciones[c[i][j]].size(); k++){
            d_euclidea =  std::abs(posiciones[c[i][j]][k] - cent[i][k]);
            d_euclidea *= d_euclidea;
            d_intra[i] += (1.0/c[i].size()) * d_euclidea;
         }
      }
   }

   desv_sol = 0;
   for( int i = 0; i < n_cluster; i++){
      desv_sol += (1.0/n_cluster)*d_intra[i];
   }

   infact = calcular_infact_sol(sol);

   evaluacion = desv_sol + lambda*infact;

   return evaluacion;
}

std::vector<int> CCP::torneo_binario(int t){
   std::vector<int> ganadores;
   int index_a, index_b;
   index_a = index_b = -1;
   for(int i = 0; i < t; i++){
      while(index_a == index_b){
         index_a = Randint(0, (int)f_generacion.size());
         index_b = Randint(0, (int)f_generacion.size());
      }
      if(f_generacion[index_a] <= f_generacion[index_b]){
         ganadores.push_back(index_a);
      } else {
         ganadores.push_back(index_b);
      }
      index_a = index_b;
   }

   return ganadores;
}

void CCP::seleccionar_mejor(){
   std::vector<int> mejor;
   int index_mejor = -1;
   double d_min = 100000000.0;
   for(int i = 0; i < (int) f_generacion.size(); i++){
      if(d_min > f_generacion[i]){
         d_min = f_generacion[i];
         index_mejor = i;
      }
   }

   mejor_generacion = generacion[index_mejor];
   f_mejor_generacion = evaluar_solucion(mejor_generacion);
}

int CCP::seleccionar_peor(){
   int index_peor = -1;
   double d_max = -100000000.0;
   for(int i = 0; i < (int) f_generacion.size(); i++){
      if(d_max < f_generacion[i]){
         d_max = f_generacion[i];
         index_peor = i;
      }
   }

   return index_peor;
}

void CCP::operador_seleccion(int t){
   std::vector<int> seleccionados = torneo_binario(t);

   for(int i = 0; i < (int) seleccionados.size(); i++){
      seleccion.push_back(generacion[seleccionados[i]]);
      f_seleccion.push_back(f_generacion[seleccionados[i]]);
   }
}

std::vector<int> CCP::operador_cruce_uniforme(std::vector<int> & p1, std::vector<int> & p2){
   int genoma = ((int) p1.size() / 2);
   int gen;
   std::set<int> genes;

   for(int i = 0; i < genoma; i++){
      gen = Randint(0,((int) p1.size()));
      auto it = genes.find(gen);
      if(it == genes.end()){
         genes.insert(gen);
      } else {
         i--;
      }
   }

   /*for(auto it = genes.begin(); it != genes.end(); ++it){
      std::cout << *it << std::endl;
   }*/

   std::vector<int> descendiente;
   for(int i = 0; i < (int) p1.size(); i++){
      auto it = genes.find(i);
      if(it != genes.end()){
         descendiente.push_back(p1[i]);
      } else {
         descendiente.push_back(p2[i]);
      }
   }
   return descendiente;
}

std::vector<int> CCP::operador_cruce_segmento(std::vector<int> & p1, double f_p1, std::vector<int> & p2, double f_p2, int n){
   int r,v,tam = (int) p1.size();
   r = Randint(0, tam);
   v = Randint(0, tam);
   std::set<int> genes_dominantes;
   int genoma = ((tam - v) / 2);
   int gen;
   std::set<int> genes;

   for(int i = 0; i < v; i++){
      genes_dominantes.insert((r+i)%tam);
   }



   for(int i = 0; i < genoma; i++){
      gen = Randint(0,tam);
      auto it = genes.find(gen);
      auto dom = genes_dominantes.find(gen);
      if(it == genes.end() && dom == genes_dominantes.end()){
         genes.insert(gen);
      } else {
         i--;
      }
   }


   std::vector<int> mejor;
   std::vector<int> peor;

   if(f_p1 <= f_p2){
      mejor = p1;
      peor = p2;
   } else {
      mejor = p2;
      peor = p1;
   }

   std::vector<int> descendiente;
   for(int i = 0; i < tam; i++){
      auto it = genes_dominantes.find(i);
      if(n == 0){
         if(it != genes_dominantes.end()){
            descendiente.push_back(mejor[i]);
         } else {
            auto it = genes.find(i);
            if(it != genes.end()){
               descendiente.push_back(p1[i]);
            } else {
               descendiente.push_back(p2[i]);
            }
         }
      } else if(n == 1){
         if(it != genes_dominantes.end()){
            descendiente.push_back(peor[i]);
         } else {
            auto it = genes.find(i);
            if(it != genes.end()){
               descendiente.push_back(p1[i]);
            } else {
               descendiente.push_back(p2[i]);
            }
         }
      }

   }
   return descendiente;
}

void CCP::operador_cruce(int n, int s,double p){
   int n_hijos = 2;
   std::vector<std::vector<int>> new_seleccion;
   std::vector<int> desc;
   int p_cruce = (s / 2) * p;

   for(int i = 0; i < p_cruce; i++){
      for(int j = 0; j < n_hijos; j++){
         if(n == 0){
            desc = operador_cruce_uniforme(seleccion[i*2],seleccion[i*2+1]);
         } else if(n == 1){
            desc = operador_cruce_segmento(seleccion[i*2],f_seleccion[i*2],seleccion[i*2+1],f_seleccion[i*2+1]);
         }
         new_seleccion.push_back(desc);
      }
   }
   for(int i = p_cruce*2; i < s; i++){
      new_seleccion.push_back(seleccion[i]);
   }

   seleccion = new_seleccion;

   mutar_seleccion();
   reparar_seleccion();

   std::vector<double> f_new_seleccion;
   for(int i = 0; i < s; i++){
      f_new_seleccion.push_back(evaluar_solucion(seleccion[i]));
   }
   f_seleccion = f_new_seleccion;
}

void CCP::conservar_elitismo(){
   auto coincidencia = std::find(generacion.begin(), generacion.end(), mejor_generacion);

   if(coincidencia == generacion.end()){
      int sustituir = seleccionar_peor();
      generacion[sustituir] = mejor_generacion;
      f_generacion[sustituir] = evaluar_solucion(mejor_generacion);
   }
}

void CCP::reparar_solucion(std::vector<int> & sol){
   bool reparacion = false;
   int aleatorio = 0;
   int tam_sol = ((int) sol.size())-1;
   std::vector<std::vector<int>> c;
   std::vector<int> c_vacio;
   c.resize(n_cluster);

   for(int i = 0; i < (int) sol.size(); i++){
      c[sol[i]].push_back(i);
   }

   for(int i = 0; i < n_cluster; i++){
      if(c[i].empty()){
         reparacion = true;
         c_vacio.push_back(i);
      }
   }
   if(reparacion){
      for(auto it = c_vacio.begin(); it != c_vacio.end(); ++it){
         aleatorio = Randint(0, tam_sol);
         while(( (int) c[sol[aleatorio]].size()) <= 1){
            aleatorio = Randint(0, tam_sol);
         }

         c[sol[aleatorio]].erase(std::find(c[sol[aleatorio]].begin(), c[sol[aleatorio]].end(),aleatorio));
         sol[aleatorio] = *it;
         c[sol[aleatorio]].push_back(aleatorio);
      }
   }
}

void CCP::reparar_seleccion(){
   for(int i = 0; i < (int) seleccion.size(); i++){
      reparar_solucion(seleccion[i]);
   }

}

void CCP::mutar_solucion(std::vector<int> & sol){
   int aleatorio, mutacion, p_mutacion = 1000;
   for(int i = 0; i < (int)sol.size(); i++){
      mutacion= Randint(1,p_mutacion);
      if(mutacion == 1){
         aleatorio = Randint(0,n_cluster);
         sol[i] = aleatorio;
      }
   }
}

void CCP::mutar_seleccion(){
   for(int i = 0; i < (int) seleccion.size(); i++){
      mutar_solucion(seleccion[i]);
   }
}

void CCP::leer_mejor_generado(){
   limpiar_clusters();
   solucion = mejor_generacion;
   for( unsigned i = 0; i < solucion.size(); i++){
      clusters[solucion[i]].push_back(i);
   }
   for( int i = 0; i < n_cluster; i++){
      calcular_centroide(i);
   }
   desviacion_general();
   infactibilidad_solucion();
   f_objetivo = desv_gen + (infactibilidad*lambda);
}

void CCP::aplicar_generacional(){
   generacion = seleccion;
   f_generacion = f_seleccion;
   conservar_elitismo();
   seleccion.clear();
   f_seleccion.clear();
}

void CCP::aplicar_estacionario(int n){
   std::vector<std::vector<int>> estacion;
   std::vector<double> f_estacion;
   std::vector<std::vector<int>> peores;
   std::vector<double> f_peores;
   int index_peor;

   double max, min;
   int i_max, i_min;

   for(int i = 0; i < n; i++){
      index_peor = seleccionar_peor();

      peores.push_back(generacion[index_peor]);
      f_peores.push_back(f_generacion[index_peor]);

      generacion.erase(std::find(generacion.begin(), generacion.end(), generacion[index_peor]));
      f_generacion.erase(std::find(f_generacion.begin(), f_generacion.end(), f_generacion[index_peor]));
   }

   std::vector<int> comprobado_peor;
   std::vector<int> comprobado_selec;

   for(int i = 0; i < n; i++){
      min = 100000000.0;
      max = -1*min;
      i_min = -1;
      i_max = i_min;
      for(int j = 0; j < n; j++){
         auto it = std::find(comprobado_selec.begin(), comprobado_selec.end(), j);
         if(f_seleccion[j] < min && it == comprobado_selec.end()){
            min = f_seleccion[j];
            i_min = j;
         }
      }
      for(int j = 0; j < n; j++){
         auto it = std::find(comprobado_peor.begin(), comprobado_peor.end(), j);
         if(f_peores[j] > max && it == comprobado_peor.end()){
            max = f_peores[j];
            i_max = j;
         }
      }

      if(f_seleccion[i_min] < f_peores[i_max]){
         estacion.push_back(seleccion[i_min]);
         f_estacion.push_back(f_seleccion[i_min]);
      } else{
         estacion.push_back(peores[i_max]);
         f_estacion.push_back(f_peores[i_max]);
      }
      comprobado_selec.push_back(i_min);
      comprobado_peor.push_back(i_max);
   }

   for(int i = 0; i < n; i++){
      generacion.push_back(estacion[i]);
      f_generacion.push_back(f_estacion[i]);
   }

   seleccion.clear();
   f_seleccion.clear();
}

bool CCP::mejor_valor(std::vector<int> & sol, double & f_sol, int i){
   double min = f_sol;
   int cluster_min = -1;
   std::vector<int> sol_comp;
   double f_sol_comp;
   std::vector<std::vector<int>> c;
   c.resize(n_cluster);
   for( int j = 0; j < (int) sol.size(); j++){
      c[sol[j]].push_back(j);
   }

   for(int j = 0; j < n_cluster; j++){
      sol_comp = sol;
      if(sol[i] != j && ((int) c[j].size()) > 1 ){
         sol_comp[i] = j;
         f_sol_comp = evaluar_solucion(sol_comp);
         if(min > f_sol_comp){
            cluster_min = j;
            min = f_sol_comp;
         }
      }
   }
   if(cluster_min == -1){
      return false;
   } else {
      sol[i] = cluster_min;
      f_sol = evaluar_solucion(sol);
      return true;
   }
}

void CCP::busqueda_local_suave(std::vector<int> & sol, double & f_sol){
   int i = 0;
   std::vector<int> rsi;
   int tam = (int) sol.size();
   bool mejora;
   int fallos;
   int umbral = 0.1*tam;
   for( int i = 0; i < tam; i++){
      rsi.push_back(i);
   }

   std::random_shuffle(rsi.begin(), rsi.end(), Randint_shuffle);

   mejora = true;
   while((mejora || fallos < umbral) && i < tam){
      if(mejor_valor(sol,f_sol,rsi[i])){
         mejora = true;
      } else {
         fallos++;
      }

      i++;
   }

}

void CCP::aplicar_BLS(double p, bool mejor){
   int probabilidad = p * ((int) generacion.size());
   std::vector<int> index_mejores;
   double f_mejor;
   int i_mejor;

   if(mejor){
      for(int i = 0; i < probabilidad; i++){
         f_mejor = 1000000.0;
         i_mejor = -1;
         for(int j = 0; j < (int) generacion.size(); j++){
            auto it = std::find(index_mejores.begin(), index_mejores.end(), j);
            if(f_generacion[j] < f_mejor && it == index_mejores.end()){
               i_mejor = j;
               f_mejor = f_generacion[j];
            }
         }
         //std::cout << "Aplico BLS al mejor: " << i_mejor << std::endl;
         index_mejores.push_back(i_mejor);
      }

      for(int i = 0; i < (int) index_mejores.size(); i++){
         busqueda_local_suave(generacion[index_mejores[i]], f_generacion[index_mejores[i]]);
      }

   } else {
      for(int i = 0; i < probabilidad; i++){
         //std::cout << "Aplico BLS: " << i << std::endl;
         busqueda_local_suave(generacion[i], f_generacion[i]);
      }
   }
}
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
int CCP::greedy(bool v){
   int i = 0, n_max = 500;
   bool cambio_c;
   std::vector<int> rsi;
   std::vector<std::vector<int>> solucion_ant = clusters;

   for( unsigned i = 0; i < posiciones.size(); i++){
      rsi.push_back(i);
   }

   std::random_shuffle(rsi.begin(), rsi.end(), Randint_shuffle);

   do {
      cambio_c = false;
      for( unsigned i = 0; i < rsi.size(); i++){
         asignar_cluster(rsi[i]);
      }
      for( int i = 0; i < n_cluster; i++){
         if(solucion_ant[i] != clusters[i]){
            calcular_centroide(i);
            cambio_c = true;
         }
      }
      solucion_ant = clusters;

      if(v){
         mostrar_solucion(true);
      }

      if(cambio_c){
         limpiar_clusters();
      }
      i++;

   } while(cambio_c && i < n_max);

   generar_solucion();
   desviacion_general();
   infactibilidad_solucion();

   f_objetivo = desv_gen + infactibilidad * lambda;
   it_greedy = i;
   return i;
}

void CCP::busqueda_local(bool v){
   double f_objetivo_ant, infactibilidad_ant;
   int i = 0;
   std::vector<int> solucion_ant;
   solucion_inicial();
   leer_solucion();
   generar_vecindario();
   f_objetivo_ant = f_objetivo;
   solucion_ant = solucion;
   infactibilidad_ant = infactibilidad;

   do{
      generar_vecino();
      i++;

      if(f_objetivo < f_objetivo_ant){
         f_objetivo_ant = f_objetivo;
         solucion_ant = solucion;
         infactibilidad_ant = infactibilidad;
         generar_vecindario();
         if(v){
            mostrar_solucion(true);
         }
      }
      else{
         solucion = solucion_ant;
         infactibilidad = infactibilidad_ant;
      }
      if(!quedan_vecinos()){
         //std::cout << "No quedan vecinos con los que probar" << std::endl;
         leer_vecino();
      }
      if(i >= 100000){
         //std::cout << "Num Max Evaluaciones" << std::endl;
      }
   }while(i < 100000 && quedan_vecinos());
   //std::cout << "Num Evaluaciones: " << i << std::endl;
   eval_bl = i;
}

void CCP::AG(int g, int n, bool v, bool graph, bool ignore_eval,int maxgen){
   int generacion = 0;
   int select = 0;
   double p_cruce = 0;
   poblacion = 50;
   int lim_eval = 100000;


   if(g == 0){
      select = poblacion;
      p_cruce = 0.7;
   } else {
      select = 2;
      p_cruce = 1;
   }

   if(ignore_eval){
      lim_eval = 0;
   }

   generacion_inicial();

   do{
      generacion++;
      seleccionar_mejor();

      if(graph){
         std::cout << generacion << " ";
         std::cout << f_mejor_generacion << std::endl;
      }

      if(v){
         std::cout << "--------------------------------------------------------------------------------------------------------------------" << std::endl;
         std::cout << "Generacion: " << generacion << std::endl;
         mostrar_generacion();
         std::cout << std::endl << std::endl;

      }

      operador_seleccion(select);
      if(v){
         std::cout << std::endl;
         std::cout << "Seleccion" << std::endl;
         mostrar_seleccion();
         std::cout << "Generacion: " << generacion << std::endl;
      }


      operador_cruce(n,select,p_cruce);
      if(v){
         std::cout << std::endl << "Cruce" << std::endl;
         mostrar_seleccion();
         std::cout << "Generacion: " << generacion << std::endl;
         std::cout << "--------------------------------------------------------------------------------------------------------------------" << std::endl;
      }

      if(g == 0){
         aplicar_generacional();
      } else {
         aplicar_estacionario(select);
      }


   }while(ind_eval < lim_eval || generacion < maxgen);
   leer_mejor_generado();
   gen_ag_am = generacion;
}

void CCP::AM(int n, double p, bool mejor, bool v, bool graph, bool ignore_eval,int maxgen){
   int generacion = 0;
   double p_cruce = 0.7;
   poblacion = 10;
   int select = poblacion;
   int lim_eval = 100000;

   if(ignore_eval){
      lim_eval = 0;
   }

   generacion_inicial();

   do{
      generacion++;
      seleccionar_mejor();

      if(graph){
         std::cout << generacion << " ";
         std::cout << f_mejor_generacion << std::endl;
      }

      if(v){
         std::cout << "--------------------------------------------------------------------------------------------------------------------" << std::endl;
         std::cout << "Generacion: " << generacion << std::endl;
         mostrar_generacion();
         std::cout << std::endl << std::endl;
      }

      operador_seleccion(select);
      if(v){
         std::cout << std::endl;
         std::cout << "Seleccion" << std::endl;
         mostrar_seleccion();
         std::cout << "Generacion: " << generacion << std::endl;
      }

      operador_cruce(0,select,p_cruce);
      if(v){
         std::cout << std::endl << "Cruce" << std::endl;
         mostrar_seleccion();
         std::cout << "Generacion: " << generacion << std::endl;
         std::cout << "--------------------------------------------------------------------------------------------------------------------" << std::endl;
      }

      aplicar_generacional();

      if(generacion%n == 0){
         aplicar_BLS(p,mejor);
      }

   }while(ind_eval < lim_eval || generacion < maxgen);//while(ind_eval < 100000 || generacion < 650);
   leer_mejor_generado();
   gen_ag_am = generacion;
}
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
void CCP::mostrar_datos(){
   std::cout << posiciones.size() << std::endl;
   for( unsigned i = 0; i < posiciones.size(); i++){
      for( unsigned j = 0; j < posiciones[i].size(); j++){
         std::cout << posiciones[i][j] << " ";
      }
      std::cout << std::endl;
   }

   std::map<std::pair<int,int>,int>::iterator it = restricciones.begin();
   for(; it != restricciones.end(); ++it){
      std::cout << (*it).first.first << ", " << (*it).first.second << ": "  << (*it).second << std::endl;
   }

   std::cout << "Restricciones: " << restricciones.size() << std::endl;

   for( unsigned i = 0; i < centroides.size(); i++){
      for( unsigned j = 0; j < centroides[i].size(); j++){
         std::cout << centroides[i][j] << " ";
      }
      std::cout << std::endl;
   }
}

void CCP::mostrar_solucion(bool completo){
   std::cout << "Funcion Objetivo: " << f_objetivo << std::endl;
   std::cout << "Desviacion general: " << desv_gen << std::endl;
   std::cout << "Infactibilidad: " << infactibilidad << std::endl;
   std::cout << "Lambda: " << lambda << std::endl;
   if(completo){
      std::cout << "Restricciones: " << restricciones.size() << std::endl;
      for( int j = 0; j < n_cluster; j++){
         std::cout << "Cluster " << j << " : ";
         for( unsigned k = 0; k < clusters[j].size(); k++){
            std::cout << clusters[j][k] << " ";
         }
         std::cout << std::endl;
      }
      std::cout << std::endl;
      for( unsigned j = 0; j < solucion.size(); j++){
         std::cout << solucion[j];
      }
      std::cout << std::endl;
      std::cout << std::endl;
   }
}

std::vector<double> CCP::fila_datos(int n){
   std::vector<double> fila;
   fila.push_back(desv_gen);
   fila.push_back(infactibilidad);
   fila.push_back(lambda);
   fila.push_back(f_objetivo);
   if(n == 0){
      fila.push_back(it_greedy);
   } else if(n == 1){
      fila.push_back(eval_bl);
   } else if(n == 2){
      fila.push_back(gen_ag_am);
      fila.push_back(ind_eval);
   }

   return fila;
}

void CCP::mostrar_generacion(){
   std::cout << "Tamaño de la poblacion: " << (int) generacion.size() << std::endl;

   for(int i = 0; i < (int) generacion.size(); i++){
      for(int j = 0; j < (int) generacion[i].size(); j++){
         std::cout << generacion[i][j];
      }
      std::cout << " F: " << f_generacion[i] << std::endl;
   }

   std::cout << "Mejor generado: ";
   for(int j = 0; j < (int) mejor_generacion.size(); j++){
      std::cout << mejor_generacion[j];
   }
   std::cout << " F: " << f_mejor_generacion << std::endl;

   std::cout << "Evaluaciones: " << ind_eval << std::endl;
}

void CCP::mostrar_seleccion(){
   std::cout << "Tamaño de la seleccion: " << (int) seleccion.size() << std::endl;
   for(int i = 0; i < (int) seleccion.size(); i++){
      for(int j = 0; j < (int) seleccion[i].size(); j++){
         std::cout << seleccion[i][j];
      }
      std::cout << " F: " << f_seleccion[i] << std::endl;
   }

   std::cout << "Mejor generado: ";
   for(int j = 0; j < (int) mejor_generacion.size(); j++){
      std::cout << mejor_generacion[j];
   }
   std::cout << " F: " << f_mejor_generacion << std::endl;

   std::cout << "Evaluaciones: " << ind_eval << std::endl;
}

void CCP::mostrar_agm(){
   std::cout << gen_ag_am << " " << f_objetivo << std::endl;
}
/////////////////////////////////////////////////////////////////////////////////
