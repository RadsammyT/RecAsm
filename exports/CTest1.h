// EXPORTED WITH RECTANGLE ASSEMBLER
// Export Type = C Array
// Required struct:
struct RecBundle { Rectangle shape; Color color; };
// You are encouraged to comment or modify this struct and the array
// Because this format is untested af and I don't know if it works in Cextern int SIZE = 6;
extern RecBundle CTest1[6] = {
RecBundle{ Rectangle{-100,50,300,50}, Color{255,251,251,255}},
RecBundle{ Rectangle{-100,150,300,50}, Color{255,251,251,255}},
RecBundle{ Rectangle{-100,250,300,50}, Color{255,251,251,255}},
RecBundle{ Rectangle{-100,350,300,50}, Color{255,251,251,255}},
RecBundle{ Rectangle{-150,50,50,350}, Color{255,251,251,255}},
RecBundle{ Rectangle{200,50,50,350}, Color{255,251,251,255}},
};
