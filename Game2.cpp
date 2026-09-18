    #include <graphics.h>
    #include <conio.h>
    #include <thread>
    #include <stdlib.h>
    #include <time.h>
    #include <windows.h> 
    #include <mmsystem.h>
    #include <cmath>
    #include <vector>
    #pragma comment(lib, "winmm.lib")

    #define pi 3.14159265358979323846
    #define MAX_Formation 5
    #define MAX_Chopper 2
    #define MAX_BULLETS 100

    class Bullet {
    public:
        int bx, by, br;
        double speed;
        bool active;
        Bullet(double startx, double starty) {
            bx = startx;
            by = starty + 20;
            br = 4;
            speed = -15;
            active = true;
        }

        void update() {
            if (!active) return;
            by += speed;
            if (by < 0) active = false;
        }

        void draw() {
            if (!active) return;
            setcolor(YELLOW);
            line(bx, by, bx, by - 12);
        }  

        bool collideswith(int x, int y, int r) {
            // Bullet-Enemy collision
            int dx = bx - x;
            int dy = by - y;
            int distanceSquared = dx * dx + dy * dy;
            int radiusSum = br + r;
            return distanceSquared <= radiusSum * radiusSum;
        }
    };

    class Enemybullet {
        public:
        int x, y, rad;
        double speedx, speedy;
        bool active;

        Enemybullet(int startx, int starty, double angle, double speed) {
            x = startx;
            y = starty;
            rad = 5;
            active = true;

            double anglerad = angle * (pi / 180.0);

            // Calculate bullet direction
            speedx = speed * cos(anglerad);
            speedy = speed * sin(anglerad);
        }

        void update() {
            if(!active) return;
            x += speedx;
            y += speedy;

            if(x < 0 || x > getmaxx() || y < 0 || y > getmaxy()) { // If the bullet go off-screen it dissapear to avoid lag
                active = false;
            }
        }

        void draw() { // Draw enemybullet
            if(!active) return;
            setcolor(RED);
            setfillstyle(SOLID_FILL, RED);
            fillellipse(x, y, rad, rad);
        }

        bool collideswithplayer(const class Player& player);
    };

    class Player {
    public:
        int aatk = 1;
        int fr = 50 / 2;
        int speed;
        int size = 50;
        int maxhp = 20;
        int currenthp;
        double x, y;
        bool prevSpaceState;
        bool isinvin;

        Bullet* bullets[MAX_BULLETS];
        Player* player;
        
        DWORD lasthit;
        DWORD isinvinduration = 1000;
        DWORD lastshot;

        void* playerimage;
        void* playermask;

        Player() : currenthp(maxhp), isinvin(false) {

            playerimage = new char[imagesize(0, 0, size, size)];
            playermask = new char[imagesize(0, 0, size, size)];

            cleardevice();
            readimagefile("D:/code raiden/Game Proposal/Recourses/playerplane.gif", 0, 0, size, size);
            getimage(0, 0, size, size, playerimage);

            cleardevice();
            readimagefile("D:/code raiden/Game Proposal/Recourses/playerplanemask.gif", 0, 0, size, size);
            getimage(0, 0, size, size, playermask);

            x = getmaxx() / 2;
            y = getmaxy() - 50;
            prevSpaceState = false; // Set spacebar push as false
            lastshot = 0;
            speed = 15; // PLayer speed

            for (int i = 0; i < MAX_BULLETS; i++) 
                bullets[i] = nullptr;
        }

        void takedmg(int dmg) {
            if(isinvin) return; // Check is the player has been damage recently

            currenthp -= dmg; // If player gets hit, it currenthp reduce
            if(currenthp < 0) currenthp = 0;

            isinvin = true;
            lasthit = GetTickCount();
             // Put hit sound here
        }

        void updateinvin() {
            if(isinvin && GetTickCount() - lasthit > isinvinduration) {
                isinvin = false; // Invinsible after a hit for 1 second
            } 
        }

        void drawhp() {
            // Draw health bar background (empty)
            setcolor(WHITE);
            rectangle(20, 50, 220, 65);  // Outline of health bar
        
            // Calculate current health width
            int healthWidth = (int)(200 * ((double)currenthp / maxhp));
        
            // Draw filled health portion
            if(currenthp > maxhp * 0.6) {
                setcolor(GREEN);
                setfillstyle(SOLID_FILL, GREEN);
            } else if(currenthp > maxhp * 0.3) {
                setcolor(YELLOW);
                setfillstyle(SOLID_FILL, YELLOW);
            } else {
                setcolor(RED);
                setfillstyle(SOLID_FILL, RED);
                }
                bar(21, 51, 20 + healthWidth, 64);
        }

        bool isalive() const {
            return currenthp > 0;
        }

        void controls() {
            if (GetAsyncKeyState('A') & 0x8000) {
                x -= speed;
                if (x < fr) x = fr;
            }
            if (GetAsyncKeyState('D') & 0x8000) {
                x += speed;
                if (x > getmaxx() - fr) x = getmaxx() - fr;
            }
            if (GetAsyncKeyState('W') & 0x8000) {
                y -= speed;
                if (y < fr) y = fr;
            }
            if (GetAsyncKeyState('S') & 0x8000) {
                y += speed;
                if (y > getmaxy() - fr) y = getmaxy() - fr;
            }

            bool spacePressedNow = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0; // Check if space is pressed

            if (spacePressedNow && !prevSpaceState) {
                shoot();
            }
            prevSpaceState = spacePressedNow;
        }

        void shoot() {
            DWORD shootcd = 230; // Shoot cooldown
            DWORD currenttime = GetTickCount();
            
            if (currenttime - lastshot < shootcd) return; // Delay per shot

            for (int i = 0; i < MAX_BULLETS; i++) {  
                if (!bullets[i]) {
                    bullets[i] = new Bullet(x, y - fr);
                    lastshot = currenttime; // Reset timer
                    break;
                }
            }
        }

        void updatebullet() {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (bullets[i]) {
                    bullets[i]->update();
                    if (!bullets[i]->active) {
                        delete bullets[i];
                        bullets[i] = nullptr;
                    }
                }
            }
        }

        void drawbullets() {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (bullets[i]) bullets[i]->draw();
            }
        }

        void draw() {
            int drawx = x - size/2;
            int drawy = y - size/2;
            putimage(drawx, drawy, playermask, AND_PUT);
            putimage(drawx, drawy, playerimage, XOR_PUT);
        }

        void update() {
            controls();
            updatebullet();
            updateinvin();
        }

        void render() {
            draw();
            drawbullets();
            drawhp();
        }

        ~Player() {
            for (int i = 0; i < MAX_BULLETS; i++)
                delete bullets[i];
        }
    };

    bool Enemybullet::collideswithplayer(const Player& player) {
        int dx = x - player.x;
        int dy = y - player.y;
        int distanceSquared = dx*dx + dy*dy;
        int radiusSum = rad + player.fr;
        return distanceSquared <= radiusSum * radiusSum;
    };

    class Chopper {
    private:
        int color;
        double speedx;
        bool active;
        bool fromRight;
        bool reachedPosition;  // Check chopper position

        const DWORD shootcd = 1700; // 1.7 Second
        DWORD lastshott;
        std::vector<Enemybullet*> bullets; 

        void* c1img;
        void* c1mask;
        void* c2img;
        void* c2mask;

    public:
        int eatk = 1;
        int chopperr;
        double chopperx, choppery;
        static Chopper* choper[MAX_Chopper];

        Chopper(bool fromRightSide, int ypos) {
            chopperr = 50;
            choppery = ypos;
            fromRight = fromRightSide;
            color = BLUE;
            active = true;
            reachedPosition = false;  // Initialize as false

            if (fromRight) { // Draw from right side 
                chopperx = getmaxx() + chopperr;
                speedx = -2;
            } else {    // Draw from left side
                chopperx = -chopperr;
                speedx = 2;
            }

            // Initiate images
            c1img = new char[imagesize(0, 0, chopperr, chopperr)];
            c2img = new char[imagesize(0, 0, chopperr, chopperr)];
            c1mask = new char[imagesize(0, 0, chopperr, chopperr)];
            c2mask = new char[imagesize(0, 0, chopperr, chopperr)];

            cleardevice();
            readimagefile("D:/code raiden/Game Proposal/Recourses/chopper1.gif", 0, 0, chopperr, chopperr);
            getimage(0, 0, chopperr, chopperr, c1img);

            cleardevice();
            readimagefile("D:/code raiden/Game Proposal/Recourses/chopper1mask.gif", 0, 0, chopperr, chopperr);
            getimage(0, 0, chopperr, chopperr, c1mask);

            cleardevice();
            readimagefile("D:/code raiden/Game Proposal/Recourses/chopper2.gif", 0, 0, chopperr, chopperr);
            getimage(0, 0, chopperr, chopperr, c2img);

            cleardevice();
            readimagefile("D:/code raiden/Game Proposal/Recourses/chopper2mask.gif", 0, 0, chopperr, chopperr);
            getimage(0, 0, chopperr, chopperr, c2mask);
        }

        void shoot() {
            // Shoot if chopper has reached position
            if (!reachedPosition) return;

            // Chopper shoot cooldown
            DWORD currenttime = GetTickCount();
            if(currenttime - lastshott < shootcd) return;

            lastshott = currenttime;

            // Shoot anggle
            bullets.push_back(new Enemybullet(chopperx, choppery, 0, 5));    // Front
            bullets.push_back(new Enemybullet(chopperx, choppery, 45, 5));   // Front-right
            bullets.push_back(new Enemybullet(chopperx, choppery, 315, 5));  // Front-left
            bullets.push_back(new Enemybullet(chopperx, choppery, 180, 5));  // Back
            bullets.push_back(new Enemybullet(chopperx, choppery, 135, 5));  // Back-right
            bullets.push_back(new Enemybullet(chopperx, choppery, 225, 5));  // Back-left
        }

        void updatebullet(Player& player) {
            for(auto it = bullets.begin(); it != bullets.end();) {
                (*it)->update();

                if((*it)->collideswithplayer(player)) { //Enemy bullets collision with player
                    player.takedmg(eatk);
                    delete *it;
                    it = bullets.erase(it); // Delete bullets if hit player
                } else if (!(*it)->active) {
                    delete *it;
                    it = bullets.erase(it);
                } else {
                    ++it;
                }
            }
        }

        void drawbullets() { // Draw chopper bullets
            for(auto bullet : bullets) {
                if(bullet && bullet->active) {  // Add null and active check
                bullet->draw();
                }
            }
        }

        void update() {
            if (!active) return;

            if (!reachedPosition) { // Move until position
                chopperx += speedx;

                // Check if in position
                if (fromRight && chopperx <= getmaxx() - 100 - chopperr) { // Position set as maxx - 100 pixels
                    reachedPosition = true;
                } else if (!fromRight && chopperx >= 100 + chopperr) { // Position set as 100 pixels
                    reachedPosition = true;
                } else if (!reachedPosition) { // If reach position shoot
                    shoot();
                }
            }

            // Still check if we've gone off screen
            if (fromRight && chopperx < -chopperr) active = false;
            if (!fromRight && chopperx > getmaxx() + chopperr) active = false;
        }

        void draw() {
        if (!active) return;

        // Calculate drawing position 
        int drawx = chopperx - chopperr/2; // Adjust for sprite width
        int drawy = choppery - chopperr/2; // Adjust for sprite height
        int tickcount = GetTickCount();

        // Draw the appropriate sprite based on direction
        if(tickcount % 2 == 0) {
            putimage(drawx, drawy, c1mask, AND_PUT);
            putimage(drawx, drawy, c1img, XOR_PUT);
        } else {
            if(c2img && c2mask) { // If we have alternate sprites
                putimage(drawx, drawy, c2mask, AND_PUT);
                putimage(drawx, drawy, c2img, XOR_PUT);
            } else { // Fallback to first sprite
                putimage(drawx, drawy, c1mask, AND_PUT);
                putimage(drawx, drawy, c1img, XOR_PUT);
            }
        }

        drawbullets();
        }

        bool isActive() const {
            return active;
        }

        bool checkBulletHit(Bullet* bullet) {
            if (!bullet || !bullet->active || !active) return false;
            return bullet->collideswith((int)chopperx, (int)choppery, chopperr); // Check if player bulllet hit chopper
        }

        // Check if the chopper hit the player
        bool checkPlayerCollision(const Player& player) {
            if (!active) return false;
        
            int dx = chopperx - player.x;
            int dy = choppery - player.y;
            int distanceSquared = dx*dx + dy*dy;
            int radiusSum = chopperr + player.fr;
        
            return distanceSquared <= radiusSum * radiusSum;
        }

        // Initiate both chopper
        static void initAll() {
            choper[0] = new Chopper(true, 200); // Spawn chopper 1
            choper[1] = new Chopper(false, 200); // Spawn chopper 2
        }

        // Update all
        static void updateAll(Player& player) {
        for (int i = 0; i < MAX_Chopper; i++) {
            if (choper[i] && choper[i]->isActive()) {

                choper[i]->update();
                choper[i]->shoot();
                choper[i]->updatebullet(player);
                choper[i]->drawbullets();
                choper[i]->draw();

                // Check player bullets
                for (int j = 0; j < MAX_BULLETS; j++) {
                    if (player.bullets[j] && choper[i]->checkBulletHit(player.bullets[j])) {
                        player.bullets[j]->active = false;
                        choper[i]->active = false;
                        break;
                    }
                }
                
                // Check player collision
                if (choper[i]->checkPlayerCollision(player)) {
                    player.takedmg(choper[i]->eatk);
                }
            }
        }

        // Draw second chopper
        if (!choper[1] && choper[0] && !choper[0]->isActive()) {
            choper[1] = new Chopper(false, 200);
        }
    }
        
        static void deleteAll() { // Delete if number reach max
            for (int i = 0; i < MAX_Chopper; i++) {
                delete choper[i];
                choper[i] = nullptr;
            }
        }
    };

    Chopper* Chopper::choper[MAX_Chopper] = { nullptr };

    class Formation {
    private:
        int color;
        int frameDelay;
        int eatk = 2;
        double speedx;
        bool active;

    public:
        bool rightdestroyed;
        bool leftdestroyed;
        bool rightActive;
        bool leftActive;
        double rx, lx, fy; // Positions
        int width, height; // Dimensions (50x20)
        static Formation* formation[MAX_Formation];

        void* flimg;
        void* flimgmask;
        void* frimg;
        void* frimgmask;

        Formation(int delayFrames, double yPosition) {
            width = 50; // Length
            height = 20; // Width
            fy = yPosition;
            rx = getmaxx() + width + 10; // Start off-screen right
            lx = -width - 10;           // Start off-screen left
            speedx = 3;
            frameDelay = delayFrames;
            color = RED;
            active = false;
            rightActive = true;
            leftActive = true;
            rightdestroyed = false;
            leftdestroyed = false;

            // Initiate images and masks
            flimg = new char[imagesize(0, 0, width, height)];
            flimgmask = new char[imagesize(0, 0, width, height)];
            frimg = new char[imagesize(0, 0, width, height)];
            frimgmask = new char[imagesize(0, 0, width, height)];

            cleardevice();
            readimagefile("D:/code raiden/Game Proposal/Recourses/formleft.gif", 0, 0, width, height );
            getimage(0, 0, width, height, flimg);

            cleardevice();
            readimagefile("D:/code raiden/Game Proposal/Recourses/formleftmask.gif", 0, 0, width, height );
            getimage(0, 0, width, height, flimgmask);

            cleardevice();
            readimagefile("D:/code raiden/Game Proposal/Recourses/formright.gif", 0, 0, width, height );
            getimage(0, 0, width, height, frimg);

            cleardevice();
            readimagefile("D:/code raiden/Game Proposal/Recourses/formrightmask.gif", 0, 0, width, height );
            getimage(0, 0, width, height, frimgmask);

        }

        // Update formation
        void update(int currentFrame) {
            if (currentFrame < frameDelay) return;
            if (!active) active = true;
        
            // Move right side
            if (rightActive) {
                rx -= speedx;
                if (rx < -width) {
                    rx = getmaxx() + width;
                }
            }

            // Move left side
            if (leftActive) {
                lx += speedx;
                if (lx > getmaxx()) {
                    lx = -width;
                }
            }
        }

        void draw(int currentFrame) {
            if (currentFrame < frameDelay) return;
            int yoffset = 42; // Set the ziz-zag patern for formation
        
            // Draw right rectangle
            if (rightActive && !rightdestroyed) {
                int drawx = rx;
                int drawy = fy;
                putimage(drawx, drawy, frimgmask, AND_PUT);
                putimage(drawx, drawy, frimg, XOR_PUT);
            }

            // Draw left rectangle
            if (leftActive && !leftdestroyed) {
                int drawx = lx;
                int drawy = fy + yoffset;
                putimage(drawx, drawy, flimgmask, AND_PUT);
                putimage(drawx, drawy, flimg, XOR_PUT);
            }
        }

        bool checkPlayerCollision(const Player& player) {
            if (!active || rightdestroyed) return false;
        
            // Rectangle-rectangle collision detection for right formation
            return (player.x + player.fr > rx) && 
                   (player.x - player.fr < rx + width) &&
                   (player.y + player.fr > fy) &&
                   (player.y - player.fr < fy + height);
        }

        bool checkPlayerCollision2(const Player& player) {
            if (!active || leftdestroyed) return false;
        
            int yoffset = 42;
            // Rectangle-rectangle collision detection for left formation
            return (player.x + player.fr > lx) && 
                   (player.x - player.fr < lx + width) &&
                   (player.y + player.fr > fy + yoffset) &&
                   (player.y - player.fr < fy + yoffset + height);
        }   

        // Check if the bullets hit right side
        bool checkRightBulletHit(Bullet* bullet) {
            if (!bullet || !bullet->active || !rightActive || rightdestroyed) return false;
        
            // Check if bullet is inside the rectangle
            if (bullet->bx > rx && 
                bullet->bx < rx + width &&
                bullet->by > fy && 
                bullet->by < fy + height) {
                rightdestroyed = true;
                return true;
            }
            return false;
        }

        bool checkLeftBulletHit(Bullet* bullet) {
            if (!bullet || !bullet->active || !leftActive || leftdestroyed) return false;
        
            int yoffset = 42;
            // Check if bullet is inside the rectangle
            if (bullet->bx > lx && 
                bullet->bx < lx + width &&
                bullet->by > fy + yoffset && 
                bullet->by < fy + yoffset + height) {
                leftdestroyed = true;
                return true;
            }
            return false;
        }

        // Initiate formation
        static void initAll() {
            for (int i = 0; i < MAX_Formation; i++) {
                double yPosition = 50 + (MAX_Formation - 1 - i) * 82;
                int delay = i * 10;
                formation[i] = new Formation(delay, yPosition);
            }
        }

        // Update all formation for bullet collision 
        static void updateAll(int frameCount, Player& player) {
            for (int i = 0; i < MAX_Formation; i++) {
                if (formation[i]) {
                    // Check for bullet collisions with right enemy
                    for (int j = 0; j < MAX_BULLETS; j++) {
                        if (player.bullets[j] && formation[i]->checkRightBulletHit(player.bullets[j])) {
                            player.bullets[j]->active = false;
                            formation[i]->rightActive = false;
                            break;
                        }
                    }
                    
                    // Check for bullet collisions with left enemy
                    for (int j = 0; j < MAX_BULLETS; j++) {
                        if (player.bullets[j] && formation[i]->checkLeftBulletHit(player.bullets[j])) {
                            player.bullets[j]->active = false;
                            formation[i]->leftActive = false;
                            break;
                        }
                    }

                    // If right formation hit player
                    if(formation[i]->checkPlayerCollision(player)) {
                        player.takedmg(formation[i]->eatk);
                    }

                    // If left formation hit player
                    if(formation[i]->checkPlayerCollision2(player)) {
                        player.takedmg(formation[i]->eatk);
                    }
                    
                    formation[i]->update(frameCount);
                    formation[i]->draw(frameCount);
                }
            }
        }

        static void deleteAll() {
            for (int i = 0; i < MAX_Formation; i++) {
                delete formation[i];
                formation[i] = nullptr;
            }
        }
    };

    Formation* Formation::formation[MAX_Formation] = { nullptr };

    class World {
    public:
        void* bufferWorld;
        int maxx, maxy;

        // Initiate background image
        World() {
            maxx = getmaxx();
            maxy = getmaxy();
            readimagefile("D:/code raiden/Game Proposal/Recourses/background.gif", 0, 0, maxx, maxy);
            bufferWorld = malloc(imagesize(0, 0, maxx, maxy));
            getimage(0, 0, maxx, maxy, bufferWorld);
            cleardevice();
        }

        // Draw background
        void drawgame() {
            putimage(0, 0, bufferWorld, COPY_PUT);
        }
    };

    // Initiate boss
    class Boss {
    private:
        int health;
        int maxhp;
        int width, height;
        double x, y;
        double speed;
        int direction; 
        int size;

        void* bossimg;
        void* bossimgmask;

        const DWORD shootcd = 1500; // 1,5 seconds
        DWORD lastshott;
        std::vector<Enemybullet*> bullets;
        int eatk = 2;

    public:
        bool active;

        Boss() {
            width = 150; 
            height = 50;
            x = getmaxx() / 2 - width / 2;
            y = 100;  
            speed = 3;  
            active = false;
            direction = 1;
            maxhp = 20;  // Set max health for boss
            health = maxhp; // Health is for current health
            lastshott = 0;
            size = 150;
            
            // Initiate image and mask
            bossimg = new char [imagesize(0, 0, size, size)];
            bossimgmask = new char [imagesize(0, 0, size, size)];

            cleardevice();
            readimagefile("D:/code raiden/Game Proposal/Recourses/bossmask.gif", 0, 0, size, size);
            getimage(0, 0, size, size, bossimgmask);

            cleardevice();
            readimagefile("D:/code raiden/Game Proposal/Recourses/boss.gif", 0, 0, size, size);
            getimage(0, 0, size, size, bossimg);
        }

        // When the boss appear this function run
        void activate() {
            active = true;
            // Play boss music when activated
            PlaySound(TEXT("D:\\code raiden\\Game Proposal\\Recourses\\bosssound.wav"), 
                 NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
            // Still error since song dissapear after another sound play
        }

        bool isActive() const {
            return active;
        }

        // Boss's shooting
        void shoot() {
            if (!active) return;

            DWORD currenttime = GetTickCount();
            if(currenttime - lastshott < shootcd) return;

            lastshott = currenttime;

            // Shoot 3 bullets downwards
            bullets.push_back(new Enemybullet(x + width/2, y + height, 90, 5));
            bullets.push_back(new Enemybullet(x + width/4, y + height, 90, 5));
            bullets.push_back(new Enemybullet(x + 3*width/4, y + height, 90, 5));
        }

        // Update bullet state
        void updatebullets(Player& player) {
            for (auto it = bullets.begin(); it != bullets.end();) {
                (*it)->update();

                if((*it)->collideswithplayer(player)) {
                    player.takedmg(eatk);
                    delete *it;
                    it = bullets.erase(it);
                }   else if (!(*it)->active) {
                    delete *it;
                    it = bullets.erase(it);
                } else {
                    it++;
                }
            }
        }

        // Draw each bullets
        void drawbullets() {
            for (auto bullet : bullets) {
                if (bullet) {
                    bullet->draw();
                }
            }
        }

        void update(Player& player) {
            if (!active) return;
            
            x += speed * direction;
            
            // Change direction when hitting screen edges
            if (x <= 0) {
                direction = 1;
                x = 0;
            } else if (x >= getmaxx() - width) {
                direction = -1;
                x = getmaxx() - width;
            }

            shoot();
            updatebullets(player);
        }

        void draw() {
            if (!active) return;
            
            // Draw boss
            int drawx = x ;
            int drawy = y - size/2;
            putimage(drawx, drawy, bossimgmask, AND_PUT);
            putimage(drawx, drawy, bossimg, XOR_PUT);
            
            // Draw healthbar at top of screen
            int screenwidth = getmaxx();
            int healthwidth = (int)((double)health / maxhp * (screenwidth - 40));
            setcolor(RED);
            setfillstyle(SOLID_FILL, RED);
            bar(20, 20, 20 + healthwidth, 30);  // Healthbar percentage
            setcolor(WHITE);
            rectangle(20, 20, screenwidth - 20, 30);  // Outline

            drawbullets();
        }

        bool checkBulletHit(Bullet* bullet) {
            if (!bullet || !bullet->active || !active) return false;
            
            // Rectangle collision
            if (bullet->bx >= x && bullet->bx <= x + width &&
                bullet->by >= y && bullet->by <= y + height) { // Check collision
                bullet->active = false;
                health--;

                if (health <= 0) {
                    active = false;

                    for (auto bullet : bullets) {
                        delete bullet;
                    }
                    bullets.clear();

                    // Play boss defeat sound
                    PlaySound(TEXT("D:/code raiden/Game Proposal/Recourses/win.wav"), 
                        NULL, SND_FILENAME | SND_ASYNC);

                    
                } else {
                    // Play hit sound
                    PlaySound(TEXT("D:/code raiden/Game Proposal/Recourses/hit.wav"), 
                            NULL, SND_FILENAME | SND_ASYNC);
                }

                return true;
            }
            
            return false;
        }
    };

    // Check if all enemies are dead
    bool allEnemiesDead() {
        // Check formations
        for (int i = 0; i < MAX_Formation; i++) {
            if (Formation::formation[i]) {
                // Consider formation dead only if both sides are destroyed or inactive
                if ((Formation::formation[i]->rightActive && !Formation::formation[i]->rightdestroyed) ||
                    (Formation::formation[i]->leftActive && !Formation::formation[i]->leftdestroyed)) {
                    return false;
                }
            }
        }  
    
        // Check choppers
        for (int i = 0; i < MAX_Chopper; i++) {
            if (Chopper::choper[i] && Chopper::choper[i]->isActive()) {
                return false;
            }
        }

        return true;
    }

    int main() {
        srand(time(0));
        initwindow(1000, 700, "Game Project");
        PlaySound(TEXT("D:/code raiden/Game Proposal/Recourses/songcom.wav"), NULL, SND_FILENAME | SND_LOOP | SND_ASYNC);

        bool gamewon = false;

        Formation::initAll();
        Chopper::initAll();
        Player player;
        World world;
        Boss boss; 
        
        int frame = 0;
        int page = 1;
        bool bossSpawned = false;

        // Initiate screen
        while (true) {
            if (kbhit()) {
                int ch = getch();
                if (ch == 27) break;
            }

            cleardevice();

            // If player dies it show game over screen
            if (!player.isalive()) {
                setcolor(RED);
                settextstyle(DEFAULT_FONT, HORIZ_DIR, 5);
                outtextxy(getmaxx()/2 - 100, getmaxy()/2, "GAME OVER");
                swapbuffers();
                delay(3000);
                break;
            }

            // Run world and player
            world.drawgame();
            player.update();
            player.render();

            Chopper::updateAll(player);
            Formation::updateAll(frame, player);
            
            // Check if all enemies are dead and spawn boss
            if (!bossSpawned && allEnemiesDead()) {
                boss.activate();
                bossSpawned = true;
            }
            
            bool bossDefeatedThisFrame = false;

            // Update and draw boss 
            if (boss.isActive()) {
                // Check bullet collisions with boss
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (player.bullets[i] && boss.checkBulletHit(player.bullets[i])) {
                        if(!boss.isActive()) {
                            bossDefeatedThisFrame = true;
                            gamewon = true;
                        }
                    }
                }
                
                boss.update(player);
                boss.draw();
            }

            if (gamewon) {
            cleardevice();
            setcolor(GREEN);
            settextstyle(DEFAULT_FONT, HORIZ_DIR, 5);
            outtextxy(getmaxx()/2 - 150, getmaxy()/2 - 50, "VICTORY!");
            
            setcolor(WHITE);
            settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
            outtextxy(getmaxx()/2 - 150, getmaxy()/2 + 50, "Press any key to exit");
            
            swapbuffers();
            
            // Wait for key press
            while (!kbhit()) {
                delay(100);
            }
            break;
        }

            delay(30);
            page = 1 - page;
            ++frame;
            swapbuffers();
        }

        getch();
        closegraph();
        Formation::deleteAll();
        Chopper::deleteAll();
        return 0;
    }