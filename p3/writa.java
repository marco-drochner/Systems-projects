import java.io.*;  
public class writa {  
public static void main(String[] args) throws Exception {     
    FileWriter writer = new FileWriter("twih.txt");  
    BufferedWriter buffer = new BufferedWriter(writer);  
    int howM = 56;
    for (int i = 0; i < howM; i++){
        buffer.write("11 ");
    }
    buffer.write("f9 17 40");
    buffer.close();  
    System.out.println("Success");  
    }  
}