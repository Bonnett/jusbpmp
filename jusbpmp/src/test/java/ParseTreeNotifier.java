import jpmp.notifier.IParseTreeNotifier;


public class ParseTreeNotifier implements IParseTreeNotifier
{

    @Override
    public long addFolder(String folderName, String mtpItemIid)
    {
        System.out.println("'" + folderName + "' " + mtpItemIid);
        return 0;
    }

    @Override
    public long addFile(String fileName, String mtpItemIid)
    {
        System.out.println(fileName);
        return 0;
    }

}
