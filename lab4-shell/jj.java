import java.util.*;
import java.io.File;  // Import the File class
import java.io.FileNotFoundException;  // Import this class to handle errors
import java.util.Scanner; // Import the Scanner class to read text files

public class jj{public static void main(String[] args) {try{

	Scanner traceO = new Scanner(new File("./daTo.txt"));
	Scanner regO = new Scanner(new File("./daRo.txt"));
	LinkedList<String> traceLs = new LinkedList<String>();
	LinkedList<String> regLs = new LinkedList<String>();
	while(traceO.hasNextLine())
	{
		traceLs.add(traceO.nextLine());
	}
	while(regO.hasNextLine())
	{
		String nextReg = regO.nextLine();
		int found = 0;
		for (int i = 0; i < traceLs.size(); i++) {
			String str = traceLs.get(i);
			if ((new StringTokenizer()))
			if (str.equals(nextReg)) {
				//already gone through and found one before?
				if (found == 1) 
					regLs.add("DUUUUUPlicate in "+nextReg);// then note
				found = 1;
				traceLs.remove(i);
			}
		}
		if (found == 0) regLs.add(nextReg);

	}
	System.out.println("\n\n-----------------Unique to trace-----------\n\n");
	for (String l : traceLs)
		System.out.println(l);
	System.out.println("\n\n--------------in regular not trace-----------\n\n");
	for (String l : regLs)
		System.out.println(l);

}catch(Exception e) {System.out.println("yo...");System.out.println(e);}
}}