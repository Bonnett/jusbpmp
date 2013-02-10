import jpmp.notifier.IParseTreeNotifier;


public class ParseTreeNotifier implements IParseTreeNotifier
{
    public long addFolder(String folderName, String mtpItemIid)
    {
        System.out.println("'" + folderName + "' " + mtpItemIid);
        return 0;
    }

    public long addFile(String fileName, String mtpItemIid)
    {
        System.out.println(fileName);
        return 0;
    }

}
