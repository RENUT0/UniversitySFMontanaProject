#include "SFApp.h"

SFApp::SFApp(std::shared_ptr<SFWindow> window) : fire(0), is_running(true), sf_window(window) {
  int canvas_w, canvas_h;
  SDL_GetRendererOutputSize(sf_window->getRenderer(), &canvas_w, &canvas_h);

  app_box = make_shared<SFBoundingBox>(Vector2(canvas_w, canvas_h), canvas_w, canvas_h);

  player  = make_shared<SFAsset>(SFASSET_PLAYER, sf_window);
  auto player_pos = Point2(canvas_w/2, 22);
  player->SetPosition(player_pos);

  background = make_shared<SFAsset>(SFASSET_BACKGROUND, sf_window);
  auto background_pos = Point2(canvas_w/2, canvas_h/2);
  background->SetPosition(background_pos);

  const int number_of_aliens = 5;
  for(int i=0; i<number_of_aliens; i++) {
    // place an alien at width/number_of_aliens * i
    auto alien = make_shared<SFAsset>(SFASSET_ALIEN, sf_window);
    auto pos   = Point2((canvas_w/number_of_aliens) * i, 225.0f);
    alien->SetPosition(pos);
    aliens.push_back(alien);
  }

  const int number_of_barriers = 8;
  for (int i=0; i<number_of_barriers; i++){
    //places two barriers on the scren
    auto barrier = make_shared<SFAsset>(SFASSET_BARRIER, sf_window);
    auto barrier2 = make_shared<SFAsset>(SFASSET_BARRIER2, sf_window);
    //bottom layer
    auto pos = Point2((canvas_w/number_of_barriers) * (i*2), 100.0f);
    barrier->SetPosition(pos);
    barriers.push_back(barrier);
    //top layer
    auto pos2 = Point2((canvas_w/(number_of_barriers+4)) * (i*2), 350.0f);
    //auto pos2 = Point2((canvas_w/(number_of_barriers+2)) * (i*2), 350.0f);
    barrier2->SetPosition(pos2);
    barriers.push_back(barrier2);
  }

  auto coin = make_shared<SFAsset>(SFASSET_COIN, sf_window);
  auto pos  = Point2((canvas_w/2), 420);
  coin->SetPosition(pos);
  coins.push_back(coin);
}

SFApp::~SFApp() {
}

/**
 * Handle all events that come from SDL.
 * These are timer or keyboard events.
 */
void SFApp::OnEvent(SFEvent& event) {
  SFEVENT the_event = event.GetCode();
  switch (the_event) {
  case SFEVENT_QUIT:
    is_running = false;
    break;
  case SFEVENT_UPDATE:
    OnUpdateWorld();
    OnRender();
    break;
  case SFEVENT_PLAYER_LEFT:
    player->GoWest();
    break;
  case SFEVENT_PLAYER_RIGHT:
    player->GoEast();
    break;
  case SFEVENT_PLAYER_UP:
    player->GoNorth();
    break;
  case SFEVENT_PLAYER_DOWN:
    player->GoSouth();
    break;
  case SFEVENT_FIRE:
    fire ++;
    FireProjectile();
    break;
  }
}

int SFApp::OnExecute() {
  // Execute the app
  SDL_Event event;
  while (SDL_WaitEvent(&event) && is_running) {
    // wrap an SDL_Event with our SFEvent
    SFEvent sfevent((const SDL_Event) event);
    // handle our SFEvent
    OnEvent(sfevent);
  }
}



int kills = 0;
int collectCoin = 0;
int e=0; //Is current step in the sequence going left to right for the coin
int w=0; //Is current step in the sequence going left to right for the aliens
int life = 3;

void SFApp::OnUpdateWorld() {
  
  auto player_pos = Point2(640/2, 22);
  
  // Update projectile positions
  for(auto p: projectiles) {
    if(p->IsAlive()) {p->GoSlow();}
  }

  // updates the coins movement
  for(auto c: coins) {
    if(e<40){
      for(int x=0;x<40;x++);{
        c->SlowEast();
	e = e+1;
      }
    }
    if(e>=40){
      for(int x=0;x<40;x++);{
        c->SlowWest();
	e = e+1;
      }
    }
    if(e==80){
      e=0;
    }
  }

  // Update enemy positions
  for(auto a : aliens) {
    if(w<260){
      for(int x=0;x<260;x++);{
        a->SlowEast();
	w = w+1;
      }
    }
    if(w>=260){
      for(int x=0;x<260;x++);{
        a->SlowWest();
	w = w+1;
      }
    }
    if(w==520){
      w=0;
    }
  }

  // Handles collisions between Projectile and Alien
  for(auto p : projectiles) {
    for(auto a : aliens) {
      if(p->CollidesWith(a)) {
        p->HandleCollision();
        a->HandleCollision();
	std::system("clear");
        kills = kills+1;
	std::cout << "You have: " << life << " lives left" << std::endl;
	std::cout << "You have: " << kills << " kills" << std::endl;
	std::cout << "You have: " << collectCoin << " coins" << std::endl;
	std::cout << "GOOD SHOT" << std::endl;
      }
    }
  }

  // handels collision between Player and Alien
  for(auto a : aliens) {
    if( player->CollidesWith(a)) {
      std::system("clear");
      life = life - 1;
      player->SetPosition(player_pos);
      std::cout << "You have: " << life << " lives left" << std::endl;
      std::cout << "You have: " << kills << " kills" << std::endl;
      std::cout << "You have: " << collectCoin << " coins" << std::endl;
      std::cout << "You crashed into an angry space orange and lost a life!!!" << std::endl;
    }
  }

  // remove dead aliens (the long way)
  list<shared_ptr<SFAsset>> tmp;
  for(auto a : aliens) {
    if(a->IsAlive()) {
      tmp.push_back(a);
    }
  }
  aliens.clear();
  aliens = list<shared_ptr<SFAsset>>(tmp);
  tmp.clear();

  // Detects collision between Player and Coin
  for(auto c : coins) {
    if( player->CollidesWith(c)){
      c->SetNotAlive();
      std::system("clear");
      collectCoin = 1;
      std::cout << "You have: " << life << " lives left" << std::endl;
      std::cout << "You have: " << kills << " kills" << std::endl;
      std::cout << "You have: " << collectCoin << " coins" << std::endl;
      std::cout << "YOU COLLECTED A COIN!" << std::endl;
    }
  }

  // detects collision between player and barrier
  for (auto b : barriers){
    if(player->CollidesWith(b)){
      std::system("clear");
      life = life - 1;
      player->SetPosition(player_pos);
      std::cout << "You have: " << life << " lives left" << std::endl;
      std::cout << "You have: " << kills << " kills" << std::endl;
      std::cout << "You have: " << collectCoin << " coins" << std::endl;
      std::cout <<"WOW, you crashed... Really?" << std::endl;
    }
  }

  // handles collision between projectile and barrier
  for (auto p : projectiles){
    for (auto b : barriers){
      if (p->CollidesWith(b)){
	p->HandleCollision();
	p->PlzStop();
      }
    }
  }

  // removes coin after collection
  for(auto c : coins){
    if(c->IsAlive()){
      tmp.push_back(c);
    }
  }
  coins.clear();
  coins = list<shared_ptr<SFAsset>>(tmp);
  tmp.clear();

  // removes projectile after collision
  for(auto p : projectiles){
    if(p->IsAlive()){
      tmp.push_back(p);
    }
  }
  projectiles.clear();
  projectiles = list<shared_ptr<SFAsset>>(tmp);
  tmp.clear();

  // displays a you lose screen if you have 0 lifes left
  if(life == 0){
    std::system("clear");
    std::cout << "You had: " << life << " lives left" << std::endl;
    std::cout << "You got: " << kills << " kills" << std::endl;
    std::cout << "You collected: " << collectCoin << " coin" << std::endl;
    std::cout << "YOU LOSE!" << std::endl;
    player->SetNotAlive();
    std::cout << "Please wait for game window to close" << std::endl;
    is_running = false;
  }

  // displays a win message if both the coin and all aliens are killed/collected
  if(kills > 4 && collectCoin == 1){
    std::system("clear");
    std::cout << "You had: " << life << " lives left" << std::endl;
    std::cout << "You got: " << kills << " kills" << std::endl;
    std::cout << "You collected: " << collectCoin << " coin" << std::endl;
    std::cout << "YOU HAVE WON!!!" << std::endl;
    is_running = false;
  }
}//end of OnUpdateWorld

void SFApp::OnRender() {
  SDL_RenderClear(sf_window->getRenderer());

  //draws the background
  background->OnRender();

  // draw the player
  if(player->IsAlive()) {player->OnRender();}

  for(auto p: projectiles) {
    if(p->IsAlive()) {p->OnRender();}
  }

  for(auto a: aliens) {
    if(a->IsAlive()) {a->OnRender();}
  }

  for(auto c: coins) {
    if(c->IsAlive()) {c->OnRender();}
  }

  for(auto b: barriers) {
    if(b->IsAlive()) {b->OnRender();}
  }

  // Switch the off-screen buffer to be on-screen
  SDL_RenderPresent(sf_window->getRenderer());
}

void SFApp::FireProjectile() {
  auto pb = make_shared<SFAsset>(SFASSET_PROJECTILE, sf_window);
  auto v  = player->GetPosition();
  pb->SetPosition(v);
  projectiles.push_back(pb);
}
