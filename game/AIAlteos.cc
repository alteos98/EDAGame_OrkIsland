#include "Player.hh"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Alteos


struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }

  /**
   * Types and attributes for your player can be defined here.
   */
   
   typedef vector<int> V;
   typedef vector<Cell> VC;
   typedef vector<V> M;
   typedef vector<VC> MC; //para definirnos el mapa
   
   V id_mis_orks; //para saber los id de nuestros orks
   M where_mis_orks; //para saber donde estan nuestros orks
   MC mapa; //matriz para cargar el mapa
   
   map<int,Pos > orks_ronda_anterior; //guardaremos la posicion de cada ork para tenerla en cuenta en la ronda siguiente
   
   int move_x[4] = { 1, -1, 0, 0 };
   int move_y[4] = { 0, 0, 1, -1 };
   
   int number_of_orks; //cantidad de orks que tenemos

  /**
   * Play method, invoked once per each round.
   */
  
   //actualiza mapa
   void actualizar_mapa (MC& mapa) {
	   for (int i=0; i < rows(); ++i) {
			  for (int j=0; j < cols(); ++j) {
				  mapa[i][j] = cell(i,j);
			  }
		}
   }
   
   //actualizamos las posiciones de los orks para tenerlos en cuenta en la proxima ronda
   void actualizar_orks_ronda_anterior(map<int,Pos>& m, vector<int>& v) {
	   for (int k=0; k < int(v.size()); ++k) {
		   m.insert(make_pair(v[k],unit(v[k]).pos));
	   }
   }
   
   //marcamos en el map de visitados las casillas con agua para que el BFS las tenga como visitadas y no siga buscando por ahí
   void marcar_agua (map<pair<int,int>, bool>& m) {
	   Cell c;
	   for (int i=0; i < rows(); ++i) {
			  for (int j=0; j < cols(); ++j) {
				  c = cell(i,j);
				  if (c.type == WATER) {
					  m.insert(make_pair(make_pair(i,j),true));
				  }
			  }
		}
   }
   
   //mover ork con identificador id en la direccion d
   void move_ork(int id, Dir d) {
		Unit u = unit(id);
		Pos pos = u.pos;
		//intentamos movernos
		pos += d;
		if (pos_ok(pos)) {
			execute(Command(id, d));
			//cerr << "ork " << id << " moved" << endl;
			return;
		}
  }
  
  //que direccion vamos en funcion de las coordenadas iniciales y finales
  //devuelve dos ya que puede ir en dos direcciones al no estar alineado con donde queremos ir
  pair<Dir,Dir> what_dir (int x_ini, int y_ini, int x_fi, int y_fi) {
	  int x = x_fi - x_ini;
	  int y = y_fi - y_ini;
	  if (x <= 0 and y > 0) {
		  return (make_pair(RIGHT,TOP));
	  }
	  if (x < 0 and y <= 0) {
		  return (make_pair(TOP,LEFT));
	  }
	  if (x >= 0 and y < 0) {
		  return (make_pair(LEFT,BOTTOM));
	  }
	  if (x > 0 and y >= 0) {
		  return (make_pair(BOTTOM,RIGHT));
	  }
	  return (make_pair(NONE,NONE));
  }
  
  //falso si ya la hemos visitiado, cierto si no
  bool pos_visitable (map<pair<int,int>, bool>& m, int x, int y) {
		bool b = true;
		map<pair<int,int>,bool>::const_iterator it = m.find(make_pair(x,y));
		if (it != m.end()) b = false;
		return b;
  }
  
  //nos devolverá la pos siguiente a la que hemos pasado, pero solo entre BOTTOM, RIGHT, TOP, LEFT.
  Dir pos_siguiente (Dir d) {
	  if (d == BOTTOM) return RIGHT;
	  else if (d == RIGHT) return TOP;
	  else if (d == TOP) return LEFT;
	  else return BOTTOM;
  }
  
  //veremos si donde nos moveremos es buena opción
  //comprobamos las casillas alrededor de nuestro ork
  Dir new_pos (int x_ini, int y_ini, Unit u, pair<Dir,Dir> pd, map<int,Pos>& orks_ronda_anterior) {
	    if (round() == 0) return pd.first;
	    map<int,Pos>::iterator it;
	    it = orks_ronda_anterior.find(u.id);
	    if (it != orks_ronda_anterior.end()) {
	  
		Dir d_else = pd.first;
		
		Dir d1 = pd.first;
		Pos pos_aux1(x_ini,y_ini);
		pos_aux1 += d1; //casilla a la que nos queremos mover
		Cell cell_aux1 = cell(pos_aux1);
		if (pos_aux1 == (*it).second) d_else = d1;
		
		Dir d2 = pd.second;
		Pos pos_aux2(x_ini,y_ini);
		pos_aux2 += d2;
		Cell cell_aux2 = cell(pos_aux2);
		if (pos_aux2 == (*it).second) d_else = d2;
		
		Dir d3 = pos_siguiente(d2);
		Pos pos_aux3(x_ini,y_ini);
		pos_aux3 += d3;
		Cell cell_aux3 = cell(pos_aux3);
		if (pos_aux3 == (*it).second) d_else = d3;
		
		Dir d4 = pos_siguiente(d3);
		Pos pos_aux4(x_ini,y_ini);
		pos_aux4 += d4;
		Cell cell_aux4 = cell(pos_aux4);
		if (pos_aux4 == (*it).second) d_else = d4;
		
		if ((cell_aux1.type != WATER) and (pos_aux1 != (*it).second) and (cell_aux1.unit_id == -1 or ((cell_aux1.unit_id > -1 and me() != unit(cell_aux1.unit_id).player) and (unit(cell_aux1.unit_id).health < u.health)))) { //donde nos moveremos no es agua o es un ork enemigo con menos vida
			return pd.first;
		}
		else if ((cell_aux2.type != WATER) and (pos_aux2 != (*it).second) and (cell_aux2.unit_id == -1 or ((cell_aux2.unit_id > -1 and me() != unit(cell_aux2.unit_id).player) and (unit(cell_aux2.unit_id).health < u.health)))) {
			return pd.second;
		}
		else if ((cell_aux3.type != WATER) and (pos_aux3 != (*it).second) and (cell_aux3.unit_id == -1 or ((cell_aux3.unit_id > -1 and me() != unit(cell_aux3.unit_id).player) and (unit(cell_aux3.unit_id).health < u.health)))) {
			return d3;
		}
		else if ((cell_aux4.type != WATER) and (pos_aux4 != (*it).second) and (cell_aux4.unit_id == -1 or ((cell_aux4.unit_id > -1 and me() != unit(cell_aux4.unit_id).player) and (unit(cell_aux4.unit_id).health < u.health)))) {
			return d4;
		}
		else {
			return d_else;
		}
		}
		else return NONE;
  }
  
  //hacemos BFS para buscar la ciudad/camino más cercano
  Dir BFS_orks (MC& mapa, Unit u, map<int,Pos>& orks_ronda_anterior) {
	  map<pair<int,int>, bool> m; //per saber quines cels hem visitat ja
	  marcar_agua(m); //marcamos como no visitables las casillas con agua
	  int x_ini = u.pos.i; //coordenada x que no modificaremos(la fila)
	  int y_ini = u.pos.j; //coordenada y que no modificaremos(la columna)
	  int x = u.pos.i; //coordenada x (la fila)
	  int y = u.pos.j; //coordenada y (la columna)
	  Cell c = cell(u.pos);
	  queue<pair<int, int> > q;
	  q.push(make_pair(x, y));
	  int contador_niveles = 1;
	  Dir d;
	  while (!q.empty()) {
		  pair<int, int> p = q.front();
		  q.pop();
		  for (int i = 0; i < 4; i++) { //las 4 posiciones de alrededor
			  x = p.first + move_x[i];
			  y = p.second + move_y[i];
			  c = cell(x,y);
			  pair<Dir,Dir> pd;
			  if (pos_ok(x,y) and c.type != WATER and pos_visitable(m,x,y)) {
				  if (contador_niveles == 1 and c.unit_id != -1) { //donde nos moveremos es un ork
						  Unit u_aux = unit(c.unit_id);
						  pd = what_dir(x_ini,y_ini,x,y);
						  if (u_aux.player != me()) { //ork NO mio
							  if (u_aux.health < u.health) { //yo tengo más vida
								//cerr << "vamos a por ork enemigo con menos vida" << endl;
								
								//d = new_pos(x_ini,y_ini,u,pd);
								d = pd.first;
								return d;
							  }
							  else { //el suyo tiene más vida o igual que yo
								  d = pd.second;
								  return d;
							  }
						  }
						  else { //ork mio
							  d = pd.second;
						  }
				  }
				  else if ((c.type == CITY and city_owner(c.city_id) != me()) or (c.type == PATH and path_owner(c.path_id) != me())) { //encontramos una ciudad o camino que no son mías
					  //cerr << u.id << " found a city/path" << endl;
					  pd = what_dir(x_ini,y_ini,x,y);
					  d = new_pos(x_ini,y_ini,u,pd,orks_ronda_anterior);
					  return d;
				  }
				  else if (contador_niveles == 2 and c.unit_id != -1) { //donde nos moveremos es un ork
						  Unit u_aux = unit(c.unit_id);
						  pd = what_dir(x_ini,y_ini,x,y);
						  if (u_aux.player != me()) { //ork NO mio
							  if (u_aux.health < u.health) { //yo tengo más vida
								//cerr << "vamos a por ork enemigo con menos vida" << endl;
								
								//d = new_pos(x_ini,y_ini,u,pd);
								d = pd.first;
								return d;
							  }
							  else { //el suyo tiene más vida o igual que yo
								  d = pd.second;
								  return d;
							  }
						  }
						  else { //ork mio
							  d = pd.second;
						  }
				  }
				  else {
					  m.insert(make_pair(make_pair(x,y),true));
					  q.push(make_pair(x, y));
				  }
			  }
		  }
		  ++contador_niveles;
	  }
	  return NONE; //si no troba cap ciutat o camí
  }
  
  virtual void play () {
	  if (round() == 0) { //primera ronda
		  number_of_orks = nb_orks();
		  mapa = MC (rows(), VC (cols()));
		  for (int i=0; i < rows(); ++i) { //cargamos el mapa completo
			  for (int j=0; j < cols(); ++j) {
				  mapa[i][j] = cell(i,j);
			  }
		  }
	  }
	  //number_of_orks = orks(me).size();
	  id_mis_orks.resize(nb_orks());
	  id_mis_orks = orks(me()); //cargamos todos los id de nuestros orks
	  cerr << "number of orks " << id_mis_orks.size() << endl;
	  //actualizar_orks_ronda_anterior(orks_ronda_anterior,id_mis_orks);
	  for (int i=0; i < int(id_mis_orks.size()); ++i) {
		  Unit u = unit(id_mis_orks[i]);
		  Dir d = BFS_orks(mapa, u, orks_ronda_anterior);
		  move_ork(id_mis_orks[i],d); //movemos orks
	  }
	  actualizar_mapa(mapa);
	  actualizar_orks_ronda_anterior(orks_ronda_anterior,id_mis_orks);
  }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
