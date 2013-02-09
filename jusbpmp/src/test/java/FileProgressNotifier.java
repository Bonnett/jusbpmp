import jpmp.notifier.IDeviceTransferNotifier;


public class FileProgressNotifier implements IDeviceTransferNotifier
{
    long totalSize; 

    @Override
    public void notifyBegin(long estimatedSize)
    {
        System.out.println("BEGIN! " + estimatedSize);
        totalSize = estimatedSize;
    }

    @Override
    public void notifyCurrent(long position)
    {
        System.out.print("Current Pos " + (position * 100 / totalSize) + "%\r\n");
    }

    @Override
    public void notifyEnd()
    {
        System.out.println("\nEND!");
    }

    @Override
    public boolean getAbort()
    {
        System.out.println("ABORT!");
        return false;
    }

}
