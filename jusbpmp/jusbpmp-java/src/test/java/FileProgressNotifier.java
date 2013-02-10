import jpmp.notifier.IDeviceTransferNotifier;


public class FileProgressNotifier implements IDeviceTransferNotifier
{
    long totalSize; 

    public void notifyBegin(long estimatedSize)
    {
        System.out.println("BEGIN! " + estimatedSize);
        totalSize = estimatedSize;
    }

    public void notifyCurrent(long position)
    {
        System.out.print("Current Pos " + (position * 100 / totalSize) + "%\r\n");
    }

    public void notifyEnd()
    {
        System.out.println("\nEND!");
    }

    public boolean getAbort()
    {
        System.out.println("ABORT!");
        return false;
    }

}
