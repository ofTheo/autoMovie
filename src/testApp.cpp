#include "testApp.h"
#include "ofxXmlSettings.h"

float NUM_SECS = 1.6;
int NUM_FRAMES = 24.0*NUM_SECS;
ofVideoPlayer vid;
ofVideoPlayer fullVid;
int framecounter = 0;
int offset = 0;
float numFrames;
float dur;
ofImage img;
int selected = 0;
float timer = 0;
int whichVideo = 0;
int totalFrames = 10000000;
bool bDown = false;
int placedIndex = 0;

class imageThumb{
	public:
		imageThumb(){
			pos = 0.0;
		}
	
		void setup(float x, float y, string path, string videoPath){
			img.loadImage(path);
			r.set(x,y,img.getWidth(),img.getHeight());
			video = videoPath;
			thumbPath = path;
			loadPos();
		}
		
		void loadPos(){
			ofxXmlSettings xml;
			if( xml.loadFile(thumbPath + ".xml") ){
				pos = xml.getValue("pos", pos);
			}
		}
		
		void savePos(){
			ofxXmlSettings xml;
			xml.setValue("pos", pos);
			xml.saveFile(thumbPath + ".xml");
		}
		
		void draw(){
			img.draw(r.x,r.y);
		}
		
		string video;
		float pos;
		string thumbPath;
		
		ofRectangle r;
		ofImage img;
};

vector <imageThumb> thumbs;
string mode = "edit";

void nextVideo(){
	if( whichVideo < thumbs.size() ){
		
		vid.loadMovie(thumbs[whichVideo].video);
		vid.play();
		vid.update();
		vid.setPosition(thumbs[whichVideo].pos);
		
		if( mode == "play" ){
			vid.setSpeed(0.25);
		}
		
		whichVideo++;
		framecounter = 0;
	}else{
		std::exit(0);
	}
}

void makeThumb(string vidPath, string thumb){
	ofVideoPlayer tmp;
	tmp.loadMovie(vidPath);
	tmp.play();
	tmp.setPosition(0.3);
	ofImage img;
	img.setFromPixels( tmp.getPixelsRef() );
	img.resize(120, 120.0f * (img.getHeight() / img.getWidth()) );
	img.saveImage(thumb);
}


void reSort(){
	float x = 10;
	float y = 10;
	
	for(int i = 0; i < thumbs.size(); i++){
		thumbs[i].r.x = x;
		thumbs[i].r.y = y;

		x += thumbs[i].r.width + 10;

		if( x + thumbs[i].r.width >= ofGetWidth() ){
			x = 10;
			y += 100;
		}
		
	}
}

void randomizeThumbs(){
	ofRandomize(thumbs);

	reSort();
}

void saveOrder(){
	ofxXmlSettings xml;
	for(int i = 0; i < thumbs.size(); i++){
		xml.setValue("videoPath", thumbs[i].video, i);
	}
	xml.saveFile("order.xml");
}

vector <string> videos;




void doFirstLoad(){

	float x = 10;
	float y = 10;
	
	thumbs.clear();
	for(int i = 0; i < videos.size(); i++){
		string name = videos[i].substr(7, videos[i].length()-7);
		string thumb = "thumbs/"+name+".jpg";
		cout <<"thumb " << thumb <<endl;
		if( !ofFile::doesFileExist(thumb) ){
			makeThumb(videos[i], thumb); 
		}
		
		imageThumb it;
		it.setup(x,y,thumb,videos[i]);
		
		if( x + it.r.width >= ofGetWidth() ){
			x = 10;
			y += 100;
		}else{
			x += it.r.width + 10;
		}
		
		thumbs.push_back(it);
	}
		
}

void loadOrder(){
	ofxXmlSettings xml;
	if( xml.loadFile("order.xml") ){
		
		videos.clear();
		int num = xml.getNumTags("videoPath");
		
		for(int i = 0; i < num; i++){
			videos.push_back( xml.getValue("videoPath", "null", i) );
		}
	
		doFirstLoad();
	}
}

void reorganizeThumbs(){
	if( selected == placedIndex ) return;
	
	vector <imageThumb> tmp;
	
	for(int i = 0; i < thumbs.size(); i++){
		if( i == placedIndex ){
			tmp.push_back( thumbs[selected] );
		}
		if( i != selected ){
			tmp.push_back( thumbs[i] );
		}
	}
	thumbs.clear();
	thumbs = tmp;
	
	reSort();
	saveOrder();
	
}

//--------------------------------------------------------------
void testApp::setup(){

	ofSetFrameRate(24);
	ofDirectory lister;
	lister.listDir("movies/");
	for(int i = 0; i < lister.size(); i++){
		videos.push_back(lister.getPath(i));
	}
	
	
	doFirstLoad();
	randomizeThumbs();
}


//--------------------------------------------------------------
void testApp::update(){
    vid.update();
	if( mode == "full" ){
		fullVid.update();
		if( fullVid.isPlaying() && ofGetElapsedTimef() > timer ){
			fullVid.setPaused(true);
		}
	}
	
}

//--------------------------------------------------------------
void testApp::draw(){
	ofBackgroundGradient(ofColor(0,0,0), ofColor(50, 50, 50), OF_GRADIENT_CIRCULAR);

	ofSetColor(255);
	if( mode == "edit" || mode == "move" ){
		if( mode == "move" ){
			ofSetColor(20, 90, 30);
			ofRect(0,0,3000,3000);
			ofSetColor(255);
		}
		
		for(int i = 0; i < thumbs.size(); i++){
			thumbs[i].draw();
		}
		
		if( mode == "move" && bDown ){
			ofSetColor(255, 190, 50);
			ofRect(thumbs[placedIndex].r.x - 5, thumbs[placedIndex].r.y, 4, 80);
		}
		
		ofSetColor(255);
		
	}else if( mode == "full" ){
		fullVid.draw(0,0);
		ofRect(thumbs[selected].pos * ofGetWidth(), ofGetHeight()-10, 4, 10);
	}else{
		
		vid.setAnchorPercent(0.5, 0.5);
		vid.draw(ofGetWidth()/2, ofGetHeight()/2, ofGetWidth(), ofGetWidth() * ( vid.getHeight() / vid.getWidth() ));
		
		if( vid.isFrameNew() ){
		
			if( mode == "play" ){
				img.grabScreen(0,0,ofGetWidth(),ofGetHeight());
				img.saveImage("frames/" + ofToString(totalFrames) + ".jpg");
			}
			totalFrames++;
			
			framecounter++;
			if( framecounter > NUM_FRAMES ){
				nextVideo();
			}		

		}
	}
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
    if(key =='e'){
		mode = "edit";
		ofSetWindowShape(1280, 900);
		reSort();
	}
	if( key == 'p' ){
		//startRender();
		mode = "play-back";
		whichVideo = 0;
		totalFrames = 0;
		framecounter = 0;		
		nextVideo();	
		ofSetWindowShape(1024, 576);

		img.setUseTexture(false);
		img.allocate(1024, 576, OF_IMAGE_COLOR);
				
	}
	if( key == 'P' ){
		//startRender();
		mode = "play";
		whichVideo = 0;
		totalFrames = 0;
		framecounter = 0;		
		nextVideo();	
		ofSetWindowShape(1024, 576);

		img.setUseTexture(false);
		img.allocate(1024, 576, OF_IMAGE_COLOR);
				
	}
	if( key == 'r' ){
		mode = "edit";
		cout << "random shuffle"<<endl;
		ofRandomize(thumbs);
	}
	if( key == 'l' ){
		loadOrder();
	}
	if( key == 'm' ){
		mode = "move";
		ofSetWindowShape(1280, 900);
		reSort();		
	}
	
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	if(mode == "full" && bDown){
		thumbs[selected].pos = ofMap(x, 0, ofGetWidth(), 0, 0.9999, true);
		fullVid.setPosition(thumbs[selected].pos);
		fullVid.update();		
	}
	else if( mode == "move" && bDown ){

		for(int i = 0; i < thumbs.size(); i++){
			ofRectangle tr = thumbs[i].r;
			if( tr.inside(x,y) ){
				placedIndex = i;
				cout << "placedIndex " << placedIndex <<endl;
			}
		}

	}
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	if( mode == "edit" ){
		for(int i = 0; i < thumbs.size(); i++){
			if(thumbs[i].r.inside(x,y)){
				mode = "full";
				selected = i;
				fullVid.loadMovie(thumbs[i].video);
				fullVid.setPosition(thumbs[i].pos);
				fullVid.update();
				break;
			}
		}
	}
	else if(mode == "full" ){
		thumbs[selected].pos = ofMap(x, 0, ofGetWidth(), 0, 0.9999, true);
		fullVid.setPosition(thumbs[selected].pos);
		fullVid.update();	
		bDown = true;			
	}

	if( mode == "move" ){
		for(int i = 0; i < thumbs.size(); i++){
			if(thumbs[i].r.inside(x,y)){
				selected = i;
				bDown = true;
				break;
			}
		}
		placedIndex = selected;
	}
	
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	if(mode == "full" && bDown){
		thumbs[selected].pos = ofMap(x, 0, ofGetWidth(), 0, 0.9999, true);
		thumbs[selected].savePos();
		fullVid.setPosition(thumbs[selected].pos);
		fullVid.update();
		fullVid.setPaused(false);		
		fullVid.play();
		timer = ofGetElapsedTimef() + NUM_SECS;		
	}
	
	if( mode == "move" ){
		if( bDown ){
			reorganizeThumbs();
		}
	}
	bDown = false;
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
