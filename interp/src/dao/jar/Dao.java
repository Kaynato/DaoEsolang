package dao.jar;

import java.io.*;


/**
 * All-in-one interpreter and "compiler."
 * @author Zicheng Gao
 */
public class Dao {

//	private static final int MAX_DEPTH = 16;

	public static void main(String[] args) {
		args = new String[]{"D:\\Cloud\\Dropbox\\J\\@@projectetc\\"
				+ "@Dao\\java_interpreter\\src\\dao\\sample\\hi2.dao"};
		if (args.length > 0) {
			File inputFile = new File(args[0]);

			String extension = getExtension(args[0]);

			// Symbol code: To compile
			if (extension.equals("dao"))
				compile(inputFile);
			// Nybble code: To run
			else if (extension.equals("wuwei")) {
				FileInputStream inputStream = openStream(inputFile);
				// TODO
				
				
				
				closeStream(inputStream);
			}


		}


	}

private static void compile(File inputFile) {
	String path = inputFile.getParent();
	String outputPath = new StringBuilder(path)
		.append('\\')
		.append(getNameWithoutExtension(inputFile.getName()))
		.append(".wuwei")
		.toString();
	File outputFile = new File(outputPath);
	FileReader inputStream = openFileReader(inputFile);
	BufferedOutputStream outputStream = openOutputStream(outputFile);

	byte isComment = 0;
	boolean emptyBuffer = true;
	byte toWrite = 0;
	char input = read(inputStream);
	
	while (input != (char)-1) {
		switch (input) {
		case '@':
			isComment++;
			break;
		case (char)0x0D:
			isComment = 0;
			break;
		case '\t':
			break;
		default:
			if (isComment == 0) {
				System.out.print(input);
				
				if (!emptyBuffer) {
					toWrite |= getNybble(input);
					write(outputStream, toWrite);
					System.out.println(byteToBinaryString(toWrite));
					emptyBuffer = true;
				}
				else {
					toWrite = (byte)(getNybble(input) << 4);
					emptyBuffer = false;
				}
			}
			break;
		}
		input = read(inputStream);
	}
	
	if (!emptyBuffer) {
		write(outputStream, toWrite);
		System.out.println("." + byteToBinaryString(toWrite));
	}

	closeStream(inputStream);
	closeStream(outputStream);
}

	private static String byteToBinaryString(byte toWrite) {
		return new StringBuilder(" ")
		.append(
		Integer.toBinaryString((toWrite & 0xFF) + (1 << 8))
		)
		.deleteCharAt(1).toString();
	}

	private static void write(BufferedOutputStream outputStream, byte toWrite) {
		try {
			outputStream.write(toWrite);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	private static byte getNybble(char input) {
		byte toWrite = 0;
		switch (input) {
		case '.':
			toWrite = 0b0000;
			break;
		case '!':
			toWrite = 0b0001;
			break;
		case '/':
			toWrite = 0b0010;
			break;
		case ']':
		case ')':
			toWrite = 0b0011;
			break;
		case '%':
			toWrite = 0b0100;
			break;
		case '#':
			toWrite = 0b0101;
			break;
		case '>':
			toWrite = 0b0110;
			break;
		case '=':
			toWrite = 0b0111;
			break;
		case '(':
			toWrite = 0b1000;
			break;
		case '<':
			toWrite = 0b1001;
			break;
		case ':':
			toWrite = 0b1010;
			break;
		case 'S':
			toWrite = 0b1011;
			break;
		case '[':
			toWrite = 0b1100;
			break;
		case '*':
			toWrite = 0b1101;
			break;
		case '$':
			toWrite = 0b1110;
			break;
		case ';':
			toWrite = 0b1111;
			break;
		}
		return toWrite;
	}

	private static char read(FileReader inputStream) {
		try {
			return (char)inputStream.read();
		} catch (IOException e) {
			System.err.println("I/O error occurred during compilation!");
			return (char)-1;
		}
	}

	private static BufferedOutputStream openOutputStream(File outputFile) {
		try {
			return new BufferedOutputStream(new FileOutputStream(outputFile));
		} catch (FileNotFoundException e) {
			System.err.println("File not found!");
			return null;
		}
	}

	private static FileReader openFileReader(File inputFile) {
		try {
			return new FileReader(inputFile);
		} catch (FileNotFoundException e) {
			System.err.println("File not found!");
			return null;
		}
	}

	private static void closeStream(Closeable stream) {
		try {
			stream.close();
		} catch (IOException e) {
			System.err.println("I/O error occurred during compilation!");
		}
	}

	private static FileInputStream openStream(File inputFile) {
		try {
			return new FileInputStream(inputFile);
		} catch (FileNotFoundException e) {
			System.err.println("File not found!");
		}
		return null;
	}

	public static String getExtension(String name) {
		StringBuilder temp = new StringBuilder(name);
		return temp.delete(0, temp.lastIndexOf(".") + 1).toString();
	}

	public static String getNameWithoutExtension(String name) {
		int stopposition = name.length() - 1;
		StringBuilder tempname = new StringBuilder(name);

		for (;tempname.charAt(stopposition) != '.'; stopposition--) {
			if (stopposition < 1)
				return name;
		}

		tempname.delete(stopposition, name.length());
		return tempname.toString();
	}

}
