type

  UnreachableError* = object of Exception
  NotImplementedError* = object of Exception

template Unreachable*(msg: string) =
  raise UnreachableError.newException(msg)

template Todo*(msg: string) =
  raise NotImplementedError.newException(msg)

