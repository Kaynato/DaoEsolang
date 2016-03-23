package dao;

public class NaivePath implements DaoInterpreter{

	/** Depth of cell size */
	protected static final int CELLDEPTH = 6;
	
	/** Bits in each cell */
	protected static final int CELL = 1 << CELLDEPTH;

	/** Data. */
	protected long[] data;

	/** Maximum depth. */
	protected final int maxDepth;

	/** Allocated depth. */
	protected int allocDepth;

	/** Allocated bits. AllocDepth ^ 2. */
	protected int alloc;

	/** Current depth. X: 0. XX: 1. XXXX: 2. */
	protected int lenDepth;

	/** Length of selection. currDepth ^ 2. */
	protected int len;

	/** Index of selection. */
	protected long index;
	
	/** Index within cell */
	protected int intIndex;
	
	/** Index within array */
	protected int arrIndex;

	/** Level. */
	protected int level;

	/**
	 * Begins a path with absolute maximum bit amount of 2^(maxDepth).<br>
	 * Minimum maxDepth is 2^6 bits.
	 * @param maxDepth Determines absolute maximum bit amount of 2^maxDepth.
	 */
	public NaivePath(int maxDepth) {
		this.maxDepth = maxDepth;
		maxDepth -= CELLDEPTH;
		data = new long[maxDepth > 0 ? 1 << maxDepth : 1];
		allocDepth = 0;
		lenDepth = 0;
		index = 0;
		alloc = 1;
		len = 1;

		level = 0;
	}

	public void IDLES() {
	}

	public void SWAPS() {	// Eager.
		if (len == 1)
			return;
		updateIndices();
		if (len <= CELL) {
			int shift = CELL - intIndex - len;
			long report = (data[arrIndex] >> shift) & mask(len);
			long left = report >> (len >> 1);
			long right = report & mask(len >> 1);
			long recombine = (right << (len >> 1)) | left;
			data[arrIndex] &= ~(mask(len) << shift);
			data[arrIndex] |= recombine << shift;
		}
		else {
			int leftIndex = arrIndex;
			int rightIndex = leftIndex + (len / CELL) - 1;
			long holder;
			while (leftIndex < rightIndex) {
				holder = data[leftIndex];
				data[leftIndex] = data[rightIndex];
				data[rightIndex] = holder;
				leftIndex++;
				rightIndex--;
			}
		}
	}

	@Override
	public void LATER() {
		if (aligned())
			index += len;
		else
			MERGE();
	}

	public void MERGE() {
		if (!aligned())
			index -= len;
		len <<= 1;
		lenDepth++;
	}

	@Override
	public void SIFTS() {
		// TODO Auto-generated method stub
		todo();
	}

	@Override
	public void EXECS() {
		byte input;
		byte toWrite;
		
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

	@Override
	public void DELEV() {
		level--;
	}

	@Override
	public void EQUAL() {
		// TODO Auto-generated method stub
		todo();
	}

	@Override
	public void HALVE() {
		if (len == 1) {
			descend();
			return;
		}
		else {
			lenDepth--;
			len = len >> 1;
		}
	}

	@Override
	public void UPLEV() {
		// TODO Auto-generated method stub
		todo();
	}

	@SuppressWarnings("unused") // CELL may be 8 if byte array is used
	@Override
	public String READS() {
		long[] report = report();
		StringBuilder output = new StringBuilder();
		if (len < 8) {
			output.append(Integer.toBinaryString(((int)report[0]) + 1 << len))
				.deleteCharAt(0);
		}
		else if (len == 8 || len == 16) {
			if (CELL >= 16)
				output.append((char)report[0]);
			else if (CELL == 8)
				output.append((char)((report[0] << 8) + report[1]));
		}
		else {
			
		}
		
		return output.toString();
			
	}

	@Override
	public void DEALC() {
		// TODO Auto-generated method stub
		todo();
	}

	@Override
	public void SPLIT() {
		if (len == 1) {
			descend();
			SPLIT();
			return;
		}
		
		updateIndices();
		if (len <= CELL) {
			int shift = CELL - intIndex - len;
			long polarized = (mask(len >> 1) << (len >> 1));
			data[arrIndex] &= ~(mask(len) << shift);
			data[arrIndex] |= polarized << shift;
		}
		else {
			int leftIndex = arrIndex;
			int rightIndex = leftIndex + (len / CELL) - 1;
			while (leftIndex < rightIndex) {
				data[leftIndex] = -1;
				data[rightIndex] = 0;
				leftIndex++;
				rightIndex--;
			}
		}

	}

	@Override
	public void POLAR() {
		// TODO Auto-generated method stub 
		todo();

	}

	@Override
	public void DOALC() {
		if (allocDepth < maxDepth) {
			alloc <<= 1;
			allocDepth++;
			MERGE();
		}
		else throw new OutOfMemoryError("Allocation exceeded specified memory.");
	}

	@Override
	public void INPUT() {
		// TODO Auto-generated method stub
		todo();
	}


	/////////////////////////////
	/////////////////////////////
	/////////////////////////////
	/////////////////////////////
	/////////////////////////////
	/////////////////////////////


	private long mask(int length) {
		if (length < CELL)
			return ((long)1 << length) - 1;
		else
			return -1;
	}

	private void descend() {
		// TODO
	}

	private boolean aligned() {
		return index % (len << 1) == 0;
	}
	
	private void todo() {
		throw new UnsupportedOperationException();
	}

	private void updateIndices() {
		arrIndex = (int)(index / CELL);
		intIndex = (int)(index % CELL);
	}
	
	private long[] report() {
		updateIndices();
		long[] output;
		if (len <= CELL) {
			int shift = CELL - intIndex - len;
			long report = ((data[arrIndex] >> shift) & mask(len));
			output = new long[]{report};
		}
		else {
			output = new long[len / CELL];
			for (int i= 0; i < output.length; i++)
				output[i] = data[arrIndex + i];
		}
		return output;
	}
	
}
