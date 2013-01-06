/* Object type enumerator */
typedef enum Objects{
	HEAD=1,
	BACK,
	TAIL,
	BULLET
}objec_t;

/* One cell structure */
struct WorldCell {
    uint32_t type;      //Šūnas tips
    uint32_t id;	//Spēlētaja id, kuram pieder objekts 
} __attribute__((packed));
