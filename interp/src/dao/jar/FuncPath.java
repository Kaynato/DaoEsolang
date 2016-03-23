package dao.jar;

import dao.jar.functional.Function;

public class FuncPath implements DaoInterpreter {

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
	public FuncPath(int maxDepth) {
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

	public final Function[] symbols = new Function[]{
		() -> IDLES(),
		() -> SWAPS(),
		() -> LATER(),
		() -> MERGE(),
		() -> SIFTS(),
		() -> EXECS(),
		() -> DELEV(),
		() -> EQUAL(),
		() -> HALVE(),
		() -> UPLEV(),
		() -> READS(),
		() -> DEALC(),
		() -> SPLIT(),
		() -> POLAR(),
		() -> DOALC(),
		() -> INPUT()
	};
	
	public void IDLES() {}

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
		
//		long[] report = reportCurrent();
		
		// TODO
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
	public void READS() {
		long[] report = reportCurrent();
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
		
		System.out.println(output.toString());
			
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
		todo();
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
	
	private long[] reportCurrent() {
		updateIndices();
		long[] output;
		if (len <= CELL)
			output = new long[]{reportInsideCell(len, arrIndex, intIndex)};
		else {
			output = new long[len / CELL];
			for (int i= 0; i < output.length; i++)
				output[i] = data[arrIndex + i];
		}
		return output;
	}
	
	protected void nybbleScan(boolean exec, boolean sift) {
		long index = this.index;
		int len = 4;
		while (index + len <= alloc) {
			int arrIndex = (int)(index / CELL);
			int intIndex = (int)(index % CELL);
			long report = reportInsideCell(len, arrIndex, intIndex);
			if (exec)
				symbols[(byte)report].pf();
			else if (sift) {
				// OH GOD WHY
			}
		}
	}

	private long reportInsideCell(int len, int arrIndex, int intIndex) {
		int shift = CELL - intIndex - len;
		long report = (data[arrIndex] >> shift) & mask(len);
		return report;
	}
	
}
